/*
 * specifreader.h
 *
 *  Created on: 3 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_IO_SPECIFREADER_H_
#define DATA_IO_SPECIFREADER_H_

#include <fstream>
#include <sstream>
#include <cstdlib> // Commands atoi, atof, srand
#include <string>
#include <time.h> // Command time
#include <vector>

#include "../misc.h"

class Specif{
public:
	/*
	 * Constructors
	 */
	Specif():_nbnodes(0),_nbedges(0),_nbpts(1),_delta(1){}
	Specif(const uint32_t& nbn, const uint32_t& nbe,const uint32_t& nbp, const uint32_t& d): _nbnodes(nbn), _nbedges(nbe), _nbpts(nbp), _delta(d){}
	Specif(const std::string& filename){
		if (filename == ""){
			ERROR("Empty specif input file name given.\n");
		}
//		STATUS("Reading specif file '" << filename << "'...");
		std::ifstream specif_file(filename);
		if ( !specif_file.is_open() ){
//			CONTINUE_STATUS(" ABORT\n");
			ERROR("Unable to open specif file.\n");
		}
		std::string paramline;
		getline(specif_file,paramline);
		_nbnodes = atoi( paramline.c_str() );
		getline(specif_file,paramline);
		_nbedges = atoi( paramline.c_str() );
		getline(specif_file,paramline);
		_nbpts = atoi( paramline.c_str() );
		getline(specif_file,paramline);
		_delta = atoi( paramline.c_str() );
		specif_file.close();
//		CONTINUE_STATUS(" OK\n");
	}

	/*
	 * Getters
	 */
	uint32_t getNbNodes() const{ return _nbnodes; }
	uint32_t getNbEdges() const{ return _nbedges; }
	uint32_t getNbPts() const{ return _nbpts; }
	uint32_t getDelta() const{ return _delta; }

	/*
	 * Setters
	 */
	void setNbNodes(const uint32_t& nbn){ _nbnodes = nbn; }
	void setNbEdges(const uint32_t& nbe){ _nbedges = nbe; }

	/*
	 * incrementEdge() method: increment the edge counter
	 */
	void incrementEdge(){ ++_nbedges; }

	/*
	 * << operator: return an outstream version of the node (printing purpose)
	 * format: (N<src>->N<dest>)[fw/bw] <distrib>
	 */
	friend std::ostream& operator<<(std::ostream& os, const Specif& specif){
		os << "Instance specification:\n" << specif._nbnodes << " nodes, " << specif._nbedges << " edges, " << specif._nbpts
				<< " support points in the discretized distributions shifted each other by " << specif._delta << " time units." << "\n";
		return os;
	}

private:
	/*
	 * Parameters
	 */
	uint32_t _nbnodes;
	uint32_t _nbedges;
	uint32_t _nbpts;
	uint32_t _delta;
};

#endif /* DATA_IO_SPECIFREADER_H_ */
