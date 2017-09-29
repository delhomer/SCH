/*
 * graphreader.h
 *
 *  Created on: 2 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_IO_GRAPHREADER_H_
#define DATA_IO_GRAPHREADER_H_

#include <fstream>
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <sstream>
#include <cstdlib> // Commands atoi, atof, srand
#include <string>
#include <time.h> // Command time
#include <vector>

#include "../misc.h"
#include "../data/graph/edge.h"
#include "specifreader.h"

class GraphReader{
public:
	/*
	 * Constructors
	 */
	GraphReader(): _hierarchized(false){}
	GraphReader(const std::string& filename, Specif* spec, const bool& hier=false): _hierarchized(hier){
		uint32_t nbPts( spec->getNbPts() );
		if (filename == ""){
			ERROR("Empty input file name given.\n");
		}
//		STATUS("Reading edge file '" << filename << "'...");
		std::ifstream input_edges(filename);
		if ( ! input_edges.is_open() ){
//			CONTINUE_STATUS(" ABORT\n");
			ERROR("Unable to open file '" << filename << "'\n");
		}
		input_edges >> std::fixed >> std::setprecision(9);
		uint32_t edge_counter(0);
		std::string line;
		while ( getline(input_edges,line) ){
			std::stringstream lineReader(line);
			++edge_counter;
			Node_id src, tgt;
			lineReader >> src;
			lineReader >> tgt;
			//			std::vector<Node_id> midnod(INVALID_NODE_ID);
			Node_id midnod_id(INVALID_NODE_ID);
			if(hier){
				//				for ( uint32_t i(0); i <= nbPts ; ++i ){
				lineReader >> midnod_id;
				//					midnod = midnod_id;
				//				}
			}
//			TRACE("Edge E" << edge_counter << ": N" << src << "-(n" << midnod_id << ")->N" << tgt );
			std::vector<uint32_t> t;
			std::vector<double> pmf,cdf;
			for ( uint32_t i = 0; i <= nbPts ; ++i ){
				uint32_t x(i * spec->getDelta() );
				t.push_back( x );
				double proba;
				lineReader >> proba;
				pmf.push_back( proba );
				cdf.push_back( proba );
				if ( x < 0 || x > nbPts * spec->getDelta() ){
					CONTINUE_STATUS(" ABORT\n");
					ERROR("Edge file corrupted: x-value not in [0;Tmax*deltat].\n");
					_edges.clear();
				}
				if ( lt(proba,0.0) || gt(proba,1.0) ){
					CONTINUE_STATUS(" ABORT\n");
					ERROR("Edge file corrupted: y-value not in [0;1].\n");
					_edges.clear();
				}
			}
			std::partial_sum(pmf.begin(),pmf.end(),cdf.begin());
			Distribution dist = Distribution(t,pmf,cdf);
//			assert( eq(cdf.back() , cdfverif.back()) );
//			assert( eq(cdfverif.back() , 1.0) );
//			assert( eq(cdf.back() , 1.0) );
			if ( neq( cdf.back() , 1.0 )  ){
				TRACE("\nLast read: " << src << "-(" << midnod_id << ")->" << tgt << ": " << cdf.back() << " " /*<< cdfverif.back() << " " */<< fabs(cdf.back() - 1.0) << " ");// << fabs(cdfverif.back() - 1.0) );
				CONTINUE_STATUS(" ABORT\n");
				ERROR("Edge file corrupted: the total of probabilities is not equal to 1. (Fmax=" << cdf.back() << ")\n");
				_edges.clear();
				break;
			}
			_edges.push_back( std::move( Edge( true , src , tgt , dist , midnod_id ) ) );
//			TRACE("Last read edge: " << _edges.back());
		}
		input_edges.close();
//		CONTINUE_STATUS(" OK\n");
	}

	/*
	 * Getter
	 */
	std::vector<Edge> getEdges(){ return _edges; }

	/*
	 * print() method: print all the saved edges on the console
	 */
	void print(){
		std::vector<Edge>::iterator ite;
		for(ite = _edges.begin() ; ite < _edges.end() ; ++ite){
			std::cout << *ite << "\n";
		}
	}

private:
	/*
	 * Parameters
	 */
	bool _hierarchized;
	std::vector<Edge> _edges;
};


#endif /* DATA_IO_GRAPHREADER_H_ */
