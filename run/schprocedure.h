/*
 * schprocedure.h
 *
 *  Created on: 3 févr. 2016
 *      Author: delhome
 */

#ifndef RUN_SCHPROCEDURE_H_
#define RUN_SCHPROCEDURE_H_

#include "run.h"

#include <algorithm>
#include <math.h>
#include <stdlib.h>

class SCHProcedure: public Run{
public:
	int main(int argc, char *argv[]){

		/*
		 * Initialization: if parameters are badly entered, terminate execution
		 */
		const char* binary_name = argv[0];
		if ( argc != 10 ){
			std::cerr << std::endl << "USAGE: " << binary_name
					<< " -p <instance name> <time-dependent scenario> <probability law> <delta> <Tmax> <instance_id> <config name> <nb_queries>"
					<< std::endl << std::endl ;
			return EXIT_FAILURE;
		}

		/*
		 * Initialization: instance specification recovering
		 */
		const std::string graph_name(argv[2]);
		const std::string tdscen(argv[3]);
		const std::string problaw(argv[4]);
		const std::string delta (argv[5]);
		const std::string tmax (argv[6]);
		const std::string instanceID (argv[7]);
		const std::string config_name(argv[8]);
		const std::string nbqueries(argv[9]);
		const std::string instance_name( graph_name + "/" +  graph_name + "_" + tdscen + "_" + problaw + "_" + delta + "_" + tmax + "_" + instanceID );
		const std::string speciffilename("instgraph/" + instance_name + "_specif.txt");
		const std::string edgefilename("instgraph/" + instance_name + ".txt");
		const std::string conf_name("instconf/" + config_name + ".cnfg");
		const std::string dmd_name("instdem/" + graph_name + "_" + nbqueries + ".dmd");
		const std::string hierarchy_name("insthier/" + instance_name + "_lvl.lvl");
		const std::string contgraph_name("insthier/" + instance_name + "_e.ctg");
		const std::string chronofilename("instchrono/chrono.txt");

		Specif* specif = new Specif( speciffilename );
		Demands* demands = new Demands( dmd_name , specif->getNbNodes() );
		Configs* configs = new Configs( conf_name );

		STATUS("Instance characterization: " << &specif << "\n");
		demands->print();

		GraphReader graphreader( edgefilename , specif );
		if ( graphreader.getEdges().empty() ){
			ERROR("Empty graph.\n");
			return EXIT_FAILURE;
		}

		/*
		 * Graph building
		 */
		Graph graph(std::move( graphreader ), *specif);

		/*
		 * Hierarchy constitution & graph contraction
		 */
		Ordering myOrdering( &graph );
		myOrdering.setConfig( configs->getFirst().getParamEq() , configs->getFirst().getParamSsd() , configs->getFirst().getParamOeq() , configs->getFirst().getParamCq() );
		auto t_orderbeg = time_stamp();
		std::vector<Edge> contGraphEdges = myOrdering.run(1);
		auto t_orderend = time_stamp();
		auto preprocessingchrono = get_duration_in_seconds(t_orderbeg, t_orderend);
		STATUS( "Preprocessing (hierarchy building+graph contraction) took " << preprocessingchrono << " sec\n");
		myOrdering.writeOrdering( hierarchy_name , contgraph_name );

		/*
		 * Query resolution
		 */
		GraphReader graphreader2( contgraph_name , specif , true );
		Graph contgraph(std::move( graphreader2 ), *specif);
		contgraph.setHierarchy( hierarchy_name );
		SCHQuery schquery = SCHQuery( &contgraph );
		std::vector<uint32_t> nbLRpaths;
		uint32_t maxLRpaths(0);
		auto t_querybeg = time_stamp();
		for(auto query: demands->getDemands() ){
			Node_id s( query.getSrc() );
			Node_id d( query.getDest() );
			RoutingPolicy result = schquery.oneToOne( s , d );
			nbLRpaths.push_back( result.getNbPaths() );
			maxLRpaths = std::max( maxLRpaths, result.getNbPaths() );
		}
		auto t_queryend = time_stamp();
		auto querychrono = get_duration_in_seconds(t_querybeg, t_queryend);
		STATUS( "Computing " << demands->getNbDmds() << " queries took " << querychrono << " sec\n");
		double meanLRpaths = round( std::accumulate(nbLRpaths.begin(), nbLRpaths.end(), 0.0)/nbLRpaths.size() * 100) / 100 ;

		/*
		 * Write results (chrono, e.g.)
		 */
		STATUS("Write a new result:\n" << graph_name << " " << tdscen << " " << problaw << " " << delta << " " << tmax << " " << instanceID << " " << nbqueries << " " );
		CONTINUE_STATUS(configs->getFirst().getParamEq() << " " << configs->getFirst().getParamSsd() << " " << configs->getFirst().getParamOeq() << " " << configs->getFirst().getParamCq() << " ");
		CONTINUE_STATUS( preprocessingchrono << " " << querychrono << " " << round( 100 * contGraphEdges.size() / (double) specif->getNbEdges() )/100 << " " << meanLRpaths << " " << maxLRpaths << "\n");
		std::ofstream chronooutput;
		chronooutput.open(chronofilename,std::ofstream::out | std::ofstream::app);
		chronooutput << graph_name << " " << tdscen << " " << problaw << " " << delta << " " << tmax << " " << instanceID << " " << nbqueries << " ";
		chronooutput << configs->getFirst().getParamEq() << " " << configs->getFirst().getParamSsd() << " " << configs->getFirst().getParamOeq() << " " << configs->getFirst().getParamCq() << " ";
		chronooutput << preprocessingchrono << " " << querychrono << " " << round( 100 * contGraphEdges.size() / (double) specif->getNbEdges() )/100 << " " << meanLRpaths << " " << maxLRpaths << "\n";

		return(EXIT_SUCCESS);
	}
};

#endif /* RUN_SCHPROCEDURE_H_ */
