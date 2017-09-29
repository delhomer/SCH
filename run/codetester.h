/*
 * codetester.h
 *
 *  Created on: 4 févr. 2016
 *      Author: delhome
 */

#ifndef RUN_CODETESTER_H_
#define RUN_CODETESTER_H_

#include "run.h"

class CodeTester: public Run{
public:
	int main(int argc, char *argv[]){

		/*
		 * Initialization: if parameters are badly entered, terminate execution
		 */
		const char* binary_name = argv[0];
		if ( argc != 2 ){
			std::cerr << std::endl << "USAGE: " << binary_name
					<< " -t"
					<< std::endl << std::endl ;
			return EXIT_FAILURE;
		}

		/*
		 * Initialization: instance specification recovering
		 */
		const std::string graph_name("grid3");
		const std::string tdscen("sta");
		const std::string problaw("dumsto");
		const std::string delta ("1");
		const std::string tmax ("20");
		const std::string instance_name( graph_name + "/" +  graph_name + "_" + tdscen + "_" + problaw + "_" + delta + "_" + tmax );
		const std::string speciffilename("instgraph/" + instance_name + "_specif.txt");
		const std::string edgefilename("instgraph/" + instance_name + "_6.txt");
		const std::string completeinstancename( instance_name + "_6.txt" );
		const std::string hierarchy_name("insthier/" + completeinstancename + "_lvl.lvl");
		const std::string contgraph_name("insthier/" + completeinstancename + "_e.ctg");
		Specif* specif = new Specif( speciffilename );

		GraphReader graphreader( edgefilename , specif );
		Graph graph(std::move( graphreader ), *specif);
		Node_id s( 0 );
		Node_id d( 5 );

		STATUS("Creation of spotar structure...");
		Spotar spotar = Spotar( &graph );
		CONTINUE_STATUS("OK\n");
		spotar.setDestination( d );
		spotar.run();
		SpotarPolicy spotarresult = spotar.getPolicy( s );

		TRACE(spotarresult);

		/*
		 * Hierarchy constitution & graph contraction
		 */
		Ordering myOrdering( &graph );
		myOrdering.setConfig( 800 , 5 , 100 , 95 );
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
		auto t_querybeg = time_stamp();
		RoutingPolicy schresult = schquery.oneToOne( s , d );
		TRACE(schresult);
		auto t_queryend = time_stamp();
		auto querychrono = get_duration_in_seconds(t_querybeg, t_queryend);
		STATUS( "Computing with SCH took " << querychrono << " sec\n");

		return(EXIT_SUCCESS);
	}
};

#endif /* RUN_CODETESTER_H_ */
