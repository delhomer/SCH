/*
 * spotarpath.cpp
 *
 *  Created on: 29 mars 2016
 *      Author: delhome
 */

#include "spotarpath.h"

/*
 * Constructors
 */
SpotarPath::SpotarPath(): _id(-1), _dsd(0), _suffixId(-1){}
SpotarPath::SpotarPath(const uint32_t& id, const Node_id& n, const int& size, const uint32_t& delta): _id(id), _dsd(size), _suffixId(-1){
	_nodes.push_back(n);
	_dist = Distribution(size,delta,false);
}
SpotarPath::SpotarPath(const SpotarPath& path): _id(path.getId()), _dsd(path.getDSD()), _suffixId(path.getSuffix()){
	_nodes = path.getNodes();
	_edges = path.getEdges();
	_dist = path.getDistribution();
}
SpotarPath::SpotarPath(const SpotarPath& path, const Node_id& n, const Edge_id& e, const Distribution& dist): _id(path.getId()), _dsd(path.getDSD()), _suffixId(path.getSuffix()){
	_nodes = path.getNodes();
	_nodes.push_back(n);
	_edges = path.getEdges();
	_edges.push_back(e);
	_dist = path.getDistribution().convolute(dist);
}
SpotarPath::SpotarPath(const std::deque<Node_id>& nodes, const std::deque<Edge_id>& edges, const Distribution& dist): _id(-1), _dsd(0), _suffixId(-1){
	_nodes = nodes;
	_edges = edges;
	_dist = dist;
}

/*
 * Getters
 */
uint32_t SpotarPath::getId() const { return _id; }
double SpotarPath::getDSD() const { return _dsd; }
uint32_t SpotarPath::getSuffix() const { return _suffixId; }
std::deque<Node_id> SpotarPath::getNodes() const { return _nodes; }
Node_id SpotarPath::getFirstNode() const { return _nodes.front(); }
Node_id SpotarPath::getLastNode() const { return _nodes.back(); }
std::deque<Edge_id> SpotarPath::getEdges() const { return _edges; }
Edge_id SpotarPath::getFirstEdge() const { return _edges.front(); }
Edge_id SpotarPath::getLastEdge() const { return _edges.back(); }
Distribution SpotarPath::getDistribution() const { return _dist; }
uint32_t SpotarPath::getShortestTime() const { return _dist.min(); }
uint32_t SpotarPath::getLargestTime() const { return _dist.max(); }
std::array<double,3> SpotarPath::getDistribT(const uint32_t& index) const{
	std::array<double,3> distribElement;
	distribElement[0] = _dist.getSupT(index);
	distribElement[1] = _dist.getPdfT(index);
	distribElement[2] = _dist.getCdfT(index);
	return distribElement;
}

/*
 * Setters
 */
void SpotarPath::setId(const uint32_t& index){ _id = index; }
void SpotarPath::setDSD(const int& degree){ _dsd = degree; }
void SpotarPath::setSuffix(const uint32_t& pathId){ _suffixId = pathId; }

/*
 * decrementDSD() method: decrement the degree of strong dominance
 */
void SpotarPath::decrementDSD(){ --_dsd; }

/*
 * incrementDSD() method: decrement the degree of strong dominance
 */
void SpotarPath::incrementDSD(){ ++_dsd; }

/*
 * appendBW(const Node_id& , const Edge_id&, const Distribution& ) method: append a node to the current path (add it to the begin of the path)
 */
SpotarPath SpotarPath::appendBW(const Node_id& node_id, const Edge_id& edge_id, const Distribution& dist){
	std::deque<Node_id> nodeseq = _nodes;
	nodeseq.push_front(node_id);
	std::deque<Edge_id> edgeseq = _edges;
	edgeseq.push_front(edge_id);
	Distribution newDist = dist.convolute( _dist );
	SpotarPath newpath = SpotarPath(nodeseq, edgeseq, newDist);
	return newpath;
}

/*
 * << operator: return an outstream version of the path (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const SpotarPath& path){
	os << "Path P" << path._id << ": N" << path._nodes[0] ;
	for(uint32_t n(1) ; n < path._nodes.size() ; ++n){
		os << " -[E" << path._edges[n-1] << "]-> N" << path._nodes[n];
	}
	os << "\t(DSD=" << path._dsd << ")\n" << path._dist;
	return os;
}


/*
 * < operator: compare two paths in terms of the visited node ids
 * e.g. Path1:1->2->5 ; Path2:1->3->4 ==> Path1 < Path2 because of the second node (as the first node is common)
 */
bool operator<(const SpotarPath& lhs, const SpotarPath& rhs){
	std::deque<Node_id> leftNodes = lhs._nodes;
	std::deque<Node_id> rightNodes = rhs._nodes;
	for( uint32_t n(0) ; n < leftNodes.size() ; ++n){
		if( n == rightNodes.size() ){ // The right-term sequence contains less nodes than the left one => inequality is wrong
			return false;
		}
		if(leftNodes[n] > rightNodes[n]){ // The right-term current node id is smaller than the left one => inequality is wrong
			return false;
		}
		if( leftNodes[n] < rightNodes[n]){ // The left-term current node id is smaller than the right one => inequality is right
			return true;
		}
	}
	// There is not any index such that node_id(left) > node_id(right) or node_id(left) < node_id(right), left-term is smaller only if its size is smaller than right-term size
	return leftNodes.size() < rightNodes.size();
}
