/*
 * querygen.h
 *
 *  Created on: 23 mars 2016
 *      Author: delhome
 */

#ifndef RUN_QUERYGEN_H_
#define RUN_QUERYGEN_H_

#include "run.h"

#include <cstdlib> // Commands atoi

class QueryGenerator: public Run{
public:
	int main(int argc, char *argv[]){

		/*
		 * Initialization: if parameters are badly entered, terminate execution
		 */
		const char* binary_name = argv[0];
		if ( argc != 8 ){
			std::cerr << std::endl << "USAGE: " << binary_name
					<< " -d <instance name> <time-dependent scenario> <probability law> <delta> <Tmax> <instance_id> <query_number>"
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
		const std::string nbQueries(argv[7]);

		const std::string instancelist( "instgraph/instancelist.txt" );

		const std::string instance_name( graph_name + "/" +  graph_name + "_" + tdscen + "_" + problaw + "_" + delta + "_" + tmax );
		const std::string speciffilename("instgraph/" + instance_name + "_specif.txt");

		Specif* specif = new Specif( speciffilename );
		uint32_t nbNodes( specif->getNbNodes() );
		uint32_t maxBudget( specif->getDelta() * specif->getNbPts() );

		int queryNumber( atoi(nbQueries.c_str()) );
		TRACE("Query number: " << queryNumber << " ; Node number: " << nbNodes << " ; budget max: " << maxBudget );

		/*
		 * Demand list building and serialization
		 */
		Demands* demands = new Demands( queryNumber , nbNodes , maxBudget );
		if( queryNumber < 0 ){
			std::stringstream queryNbStream;
			queryNbStream << "instdem/" << graph_name << "_" << nbNodes*(nbNodes-1) << ".dmd";
			const std::string dmdfilename( queryNbStream.str() );
			STATUS("Write " << dmdfilename << " demand file.\n");
			demands->serialize( dmdfilename );
			const std::string dmdfilename2("instdem/" + graph_name + "_" + nbQueries + ".dmd");
			demands->serialize( dmdfilename2 );
		}
		else{
			const std::string dmdfilename("instdem/" + graph_name + "_" + nbQueries + ".dmd");
			demands->serialize( dmdfilename );
		}

		return(EXIT_SUCCESS);
	}
};

#endif /* RUN_QUERYGEN_H_ */
