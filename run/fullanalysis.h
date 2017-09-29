/*
 * fullanalysis.h
 *
 *  Created on: 24 mars 2016
 *      Author: delhome
 */

#ifndef RUN_FULLANALYSIS_H_
#define RUN_FULLANALYSIS_H_

#include <string>

#include "run.h"

class FullAnalysis: public Run {
public:
	int main(int argc, char *argv[]){

		//		std::ofstream algo_output_spotar;
		//		std::ofstream algo_output_sch;
		//		algo_output_spotar.open("algo_output_SPOTAR.txt",std::ofstream::out | std::ofstream::app);
		//		algo_output_sch.open("algo_output_SCH.txt",std::ofstream::out | std::ofstream::app);
		//		std::ofstream algo_output_spotar("algo_output_SPOTAR.txt");
		//		std::ofstream algo_output_sch("algo_output_SCH.txt");

		/*
		 * Initialization: if parameters are badly entered, terminate execution
		 */
		const char* binary_name = argv[0];
		if ( argc != 6 ){
			std::cerr << std::endl << "USAGE: " << binary_name
					<< " -f <instfilename> <config name> <nb_queries> <result_file>"
					<< std::endl << std::endl ;
			return EXIT_FAILURE;
		}
		const std::string instfilename(argv[2]);
		const std::string configfilename(argv[3]);
		const std::string conf_name("instconf/" + configfilename + ".cnfg");
		const std::string nbqueries(argv[4]);
		const std::string chronofilename(argv[5]);
		const std::string chronocopyfilename( "D:/delhome/Dropbox/These/Workspace_Eclipse/SCH/instchrono/chronocopy.txt" );
		/*
		 * Read instance names into a dedicated file
		 */
//		STATUS("Try to read instance list file " << instfilename << "...");
		std::ifstream instfile( instfilename );
		if ( !instfile.is_open() ){
//			CONTINUE_STATUS(" ABORT\n");
			ERROR("Unable to open instance file.\n");
		}
//		CONTINUE_STATUS(" OK\n");
		std::string instline;
		/*
		 * Compute the Contraction Hierarchiy procedure for each used graph
		 */
		while( getline(instfile, instline) ){
			std::stringstream param(instline);
			std::string graph_name, tdscen, problaw, delta, tmax, varscen, id;
			param >> graph_name;
			const std::string dmd_name("instdem/" + graph_name + "_" + nbqueries + ".dmd");
			param >> tdscen;
			param >> delta;
			param >> tmax;
			param >> problaw;
			param >> varscen;
			param >> id;
			const std::string instance_name( graph_name + "/" +  graph_name + "_" + tdscen + "_" + delta + "_" + tmax );
			const std::string speciffilename("instgraph/" + instance_name + "_specif.txt");
			const std::string completeinstancename( instance_name + "_" + problaw + "_" + varscen + "_" + id );
			MARK("Instance: " << completeinstancename);
			/*
			 * Recover full instance names thanks to instance IDs
			 */
			const std::string edgefilename("instgraph/" + completeinstancename + ".txt");
			const std::string hierarchy_name("insthier/" + completeinstancename + "_lvl.lvl");
			const std::string contgraph_name("insthier/" + completeinstancename + "_e.ctg");
			/*
			 * Instance characterization
			 */
			Specif* specif = new Specif( speciffilename );
			Demands* demands = new Demands( dmd_name , specif->getNbNodes() );
			Configs* configs = new Configs( conf_name );
			//					MARK("Instance characterization: " << *specif );
			/*
			 * Read edges into the corresponding .txt file and build graph
			 */
			GraphReader graphreader( edgefilename , specif );
			if ( graphreader.getEdges().empty() ){
				ERROR("Empty graph.\n");
			}
			else{
				//						algo_output_spotar << "=========================== " << completeinstancename << " ===========================\n";
				//						algo_output_sch << "=========================== " << completeinstancename << " ===========================\n";
				Graph graph(std::move( graphreader ), *specif);
				std::vector<uint32_t> nbLRpaths_sch, nbLRpaths_spotar;
				std::vector<SpotarPolicy> result_spotar;
				std::vector<RoutingPolicy> result_sch;
				uint32_t maxLRpaths_sch(0), maxLRpaths_spotar(0);
				auto demandlist = demands->getDemands();
				STATUS("Beginning of the following instance: " << completeinstancename << "\n");
				/*
				 * Query resolution through Spotar-LC algorithm (see Nie&Wu, 2009)
				 */
				Spotar spotar( &graph );
				auto t_spotarbeg = time_stamp();
				for(int queryID(0) ; queryID < demands->getNbQueries() ; ++queryID ){
					std::pair<Node_id,Node_id> od = demands->getOD( queryID );
					Node_id s( std::get<0>(od) );
					Node_id d( std::get<1>(od) );
					//							STATUS("Path query between Node N" << s << " and Node N" << d << "");
					spotar.setDestination( d );
					spotar.run();
					SpotarPolicy result = spotar.getPolicy( s );
					//							TRACE(result);
					//							algo_output_spotar << "Path query between Node N" << s << " and Node N" << d << "\n" << result;
					result_spotar.push_back(result);
					spotar.reset();
					nbLRpaths_spotar.push_back( result.getNbLRPaths() );
					maxLRpaths_spotar = std::max( maxLRpaths_spotar, result.getNbLRPaths() );
					//							CONTINUE_STATUS(" => " << result.getNbLRPaths() << " path(s)\tOK\n");
				}
				auto t_spotarend = time_stamp();
				auto spotarchrono = get_duration_in_seconds(t_spotarbeg, t_spotarend);
				STATUS( "Computing " << demands->getNbDmds() << " queries with SPOTAR-LC took " << spotarchrono << " sec\n");
				double meanLRpaths_spotar = round( std::accumulate(nbLRpaths_spotar.begin(), nbLRpaths_spotar.end(), 0.0)/nbLRpaths_spotar.size() * 1000) / 1000 ;
				/*
				 * Hierarchy constitution & graph contraction
				 */
				Ordering myOrdering( &graph );
				myOrdering.setConfig( configs->getFirst().getParamEq() , configs->getFirst().getParamSsd() , configs->getFirst().getParamOeq() , configs->getFirst().getParamCq() );
				auto t_orderbeg = time_stamp();
				std::vector<Edge> contGraphEdges = myOrdering.run( 3 ); // Run ordering with 3 threads
				auto t_orderend = time_stamp();
				auto preprocessingchrono = get_duration_in_seconds(t_orderbeg, t_orderend);
				myOrdering.writeOrdering( hierarchy_name , contgraph_name );
				/*
				 * Query resolution
				 */
				GraphReader graphreader2( contgraph_name , specif , true );
				Graph contgraph(std::move( graphreader2 ), *specif);
				contgraph.setHierarchy( hierarchy_name );
				SCHQuery schquery = SCHQuery( &contgraph );
				auto t_querybeg = time_stamp();
				for(auto queryID(0) ; queryID < demands->getNbQueries() ; ++queryID ){
					std::pair<Node_id,Node_id> od = demands->getOD( queryID );
					Node_id s( std::get<0>(od) );
					Node_id d( std::get<1>(od) );
//												STATUS("Path query between Node N" << s << " and Node N" << d << "\n");
					RoutingPolicy result = schquery.oneToOne( s , d );
					//							TRACE(result);
					//							algo_output_sch << "Path query between Node N" << s << " and Node N" << d << "\n" << result;
//					result_sch.push_back(result);
					nbLRpaths_sch.push_back( result.getNbPaths() );
					maxLRpaths_sch = std::max( maxLRpaths_sch, result.getNbPaths() );
//												CONTINUE_STATUS(" => " << result.getNbPaths() << " path(s)\tOK\n");
				}
				auto t_queryend = time_stamp();
				auto querychrono = get_duration_in_seconds(t_querybeg, t_queryend);
				STATUS( "Computing " << demands->getNbDmds() << " queries with SCH took " << querychrono << " sec\n");
				double meanLRpaths_sch = round( std::accumulate(nbLRpaths_sch.begin(), nbLRpaths_sch.end(), 0.0)/nbLRpaths_sch.size() * 1000) / 1000 ;
				/*
				 * Write results (chrono, e.g.)
				 */
				STATUS("Write a new result:\t" << graph_name << " " << tdscen << " " << delta << " " << tmax << " " << problaw << " " << varscen << " " << id << " " << nbqueries << " " );
				CONTINUE_STATUS(configs->getFirst().getParamEq() << " " << configs->getFirst().getParamSsd() << " " << configs->getFirst().getParamOeq() << " " << configs->getFirst().getParamCq() << " ");
				CONTINUE_STATUS( preprocessingchrono << " " << round( 100 * contGraphEdges.size() / (double) specif->getNbEdges() )/100 << " " << querychrono << " " << meanLRpaths_sch << " " << maxLRpaths_sch << " ") ;
				CONTINUE_STATUS( spotarchrono << " " << meanLRpaths_spotar << " " << maxLRpaths_spotar << "\n\n");
				std::ofstream chronooutput;
				chronooutput.open(chronofilename,std::ofstream::out | std::ofstream::app);
				chronooutput << graph_name << " " << tdscen << " " << delta << " " << tmax << " " << problaw << " " << varscen << " "<< id << " " << nbqueries << " ";
				chronooutput << configs->getFirst().getParamEq() << " " << configs->getFirst().getParamSsd() << " " << configs->getFirst().getParamOeq() << " " << configs->getFirst().getParamCq() << " ";
				chronooutput << preprocessingchrono << " " << round( 100 * contGraphEdges.size() / (double) specif->getNbEdges() )/100 << " " << querychrono << " " << meanLRpaths_sch << " " << maxLRpaths_sch << " ";
				chronooutput << spotarchrono << " " << meanLRpaths_spotar << " " << maxLRpaths_spotar << "\n";

				std::ofstream chronooutputcopy;
				chronooutputcopy.open(chronocopyfilename,std::ofstream::out | std::ofstream::app);
				chronooutputcopy << graph_name << " " << tdscen << " " << delta << " " << tmax << " " << problaw << " " << varscen << " "<< id << " " << nbqueries << " ";
				chronooutputcopy << configs->getFirst().getParamEq() << " " << configs->getFirst().getParamSsd() << " " << configs->getFirst().getParamOeq() << " " << configs->getFirst().getParamCq() << " ";
				chronooutputcopy << preprocessingchrono << " " << round( 100 * contGraphEdges.size() / (double) specif->getNbEdges() )/100 << " " << querychrono << " " << meanLRpaths_sch << " " << maxLRpaths_sch << " ";
				chronooutputcopy << spotarchrono << " " << meanLRpaths_spotar << " " << maxLRpaths_spotar << "\n";

//				for(uint32_t index(0) ; index < nbLRpaths_spotar.size() ; ++index){
//					std::pair<Node_id,Node_id> od = demands->getOD( index );
//					Node_id s( std::get<0>(od) );
//					Node_id d( std::get<1>(od) );
//					std::stringstream rp1namestream, rp2namestream;
//					rp1namestream << "routingpolicy_examples/" << completeinstancename << "_" << s << "_" << d << "_RP.txt";
//					rp2namestream << "routingpolicy_examples/" << completeinstancename << "_" << s << "_" << d << "_PN.txt";
//					std::string rpfilename( rp1namestream.str() );
//					std::string pnfilename( rp2namestream.str() );
//					result_sch[index].serialize( rpfilename , pnfilename );
					//							if( nbLRpaths_spotar[index] != nbLRpaths_sch[index] ){
					//								algo_output_spotar << " ****** Path query between Node N" << s << " and Node N" << d << " ****** \n" << result_spotar[index];
					//								algo_output_sch << " ****** Path query between Node N" << s << " and Node N" << d << " ****** \n" << result_sch[index];
					//							}
//				}
			}
		}
		instfile.close();
		//		algo_output_sch.close();
		//		algo_output_spotar.close();
		return(EXIT_SUCCESS);
	}
};

#endif /* RUN_FULLANALYSIS_H_ */
