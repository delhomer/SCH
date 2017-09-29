/*
 * hierarchy_io.h
 *
 *  Created on: 26 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_IO_HIERARCHY_IO_H_
#define DATA_IO_HIERARCHY_IO_H_

#include "../misc.h"

class HierarchyIO{
public:
	/*
	 * Constructors
	 */
	HierarchyIO(): _hierarchyFile(){}
	HierarchyIO(const std::string& filename, const std::vector<Node_id>& nodes, const std::vector<uint32_t>& levels){
		_hierarchyFile = filename;
		_nodes = nodes;
		_levels = levels;
	}

	/*
	 * Getters
	 */
	std::vector<Node_id> getNodes() const { return _nodes;}
	Node_id getNode(const uint32_t& index) const { return _nodes[index];}
	std::vector<uint32_t> getLevels() const { return _levels;}
	uint32_t getLevel(const uint32_t& index) const { return _levels[index];}

	/*
	 * read() method:
	 */
	void read(){
		if (_hierarchyFile == ""){
			ERROR("Empty hierarchy file name given.\n");
		}
//		STATUS("Reading hierarchy file '" << _hierarchyFile << "'...");
		std::ifstream hierarchyStream(_hierarchyFile);
		if ( ! hierarchyStream.is_open() ){
//			CONTINUE_STATUS(" ABORT\n");
			ERROR("Unable to open hierarchy file.\n");
		}
		std::string paramline, param;
		int counter(0);
		std::string line;
		while( getline(hierarchyStream,line) ){
			std::stringstream lineReader(line);
			Node_id node, level;
			lineReader >> node;
			_nodes[counter] = node ;
			lineReader >> level;
			_levels[counter] = level;
//			TRACE("Last read: Node N" << node << " has level L" << counter << "; Level L" << level << " is occupied by node N" << counter);
			++counter;
		}
		hierarchyStream.close();
//		CONTINUE_STATUS(" OK\n");
	}

	/*
	 * write() method:
	 */
	void write(){
//		STATUS("Writing hierarchy file '" << _hierarchyFile << "'...");
		std::ofstream hierarchyOut;
		hierarchyOut.open(_hierarchyFile);
		std::vector<Node_id>::iterator itn = _nodes.begin();
		std::vector<uint32_t>::iterator itl = _levels.begin();
		for( ; itn < _nodes.end() ; ++itn, ++itl){
			hierarchyOut << *itn << "\t" << *itl << "\n";
		}
		hierarchyOut.close();
//		CONTINUE_STATUS(" OK\n");
	}

	/*
	 * saveHierarchy(std::vector<Node_id>&, std::vector<uint32_t>&) method: copy the _nodes and _levels attributes into given vectors
	 */
	void recoverHierarchy(std::vector<Node_id>& nodes, std::vector<uint32_t>& levels){
		for(uint32_t t(0) ; t < nodes.size() ; ++t){
			nodes[t] = _nodes[t];
			levels[t] = _levels[t];
		}
	}

private:
	/*
	 * Parameters
	 */
	std::vector<Node_id> _nodes; // Nodes id ranked by level
	std::vector<uint32_t> _levels; // Levels ranked by node ids
	std::string _hierarchyFile;
};

#endif /* DATA_IO_HIERARCHY_IO_H_ */
