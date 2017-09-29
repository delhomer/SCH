/*
 * dyngraphreader.h
 *
 *  Created on: 28 avr. 2016
 *      Author: delhome
 */

#ifndef DATA_IO_DYNGRAPHREADER_H_
#define DATA_IO_DYNGRAPHREADER_H_

#include <fstream>
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <sstream>
#include <cstdlib> // Commands atoi, atof, srand
#include <string>
#include <time.h> // Command time
#include <vector>

#include "../misc.h"
#include "../data/graph/dynedge.h"
#include "specifreader.h"

class DynGraphReader{
public:
	/*
	 * Constructors
	 */
	DynGraphReader(){}
	DynGraphReader(const std::string& filename, Specif* spec){
		uint32_t nbPts( spec->getNbPts() );
		if (filename == ""){
			ERROR("Empty input file name given.\n");
		}
		std::ifstream input_edges(filename);
		if ( ! input_edges.is_open() ){
			ERROR("Unable to open file '" << filename << "'\n");
		}
		input_edges >> std::fixed >> std::setprecision(9);
		uint32_t edge_counter(0);
		std::string line;
		std::vector<Distribution> weight;
		Node_id weightsrc(INVALID_NODE_ID), weighttgt(INVALID_NODE_ID);
		uint32_t datecheck(-1);
		while ( getline(input_edges,line) ){
			std::stringstream lineReader(line);
			++edge_counter;
			Node_id src, tgt;
			uint32_t date;
			lineReader >> src;
			lineReader >> tgt;
			lineReader >> date;
			assert( date == datecheck+1);
			++datecheck;
			Node_id midnod_id(INVALID_NODE_ID);
			if( src != weightsrc || tgt != weighttgt ){
				assert( weight.size() == spec->getNbPts() );
				if(weighttgt != INVALID_NODE_ID && weightsrc != INVALID_NODE_ID){
					_edges.push_back( std::move( DynEdge( true , weightsrc , weighttgt , weight , midnod_id ) ) );
				}
				weightsrc = src;
				weighttgt = tgt;
				weight.clear();
			}
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
			weight.push_back(dist);
			if ( neq( cdf.back() , 1.0 )  ){
				TRACE("\nLast read: " << src << "-(" << midnod_id << ")->" << tgt << ": " << cdf.back() << " " /*<< cdfverif.back() << " " */<< fabs(cdf.back() - 1.0) << " ");// << fabs(cdfverif.back() - 1.0) );
				CONTINUE_STATUS(" ABORT\n");
				ERROR("Edge file corrupted: the total of probabilities is not equal to 1. (Fmax=" << cdf.back() << ")\n");
				_edges.clear();
				break;
			}
		}
		input_edges.close();
	}

	/*
	 * Getter
	 */
	std::vector<DynEdge> getEdges(){ return _edges; }

	/*
	 * print() method: print all the saved edges on the console
	 */
	void print(){
		std::vector<DynEdge>::iterator ite;
		for(ite = _edges.begin() ; ite < _edges.end() ; ++ite){
			std::cout << *ite << "\n";
		}
	}

private:
	/*
	 * Parameters
	 */
	std::vector<DynEdge> _edges;
};

#endif /* DATA_IO_DYNGRAPHREADER_H_ */
