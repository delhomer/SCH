/*
 * path.cpp
 *
 *  Created on: 2 mars 2016
 *      Author: delhome
 */

#include "path.h"
#include <map>
#include <stack>

/*
 * Constructors
 */
Path::Path(): _id(-1), _dsd(0){ }
Path::Path(const uint32_t& id, const Node_id& n, const int& size, const uint32_t& delta): _id(id), _dsd(size){
	_nodes.push_back(n);
	_dist = Distribution(size,delta,false);
}
Path::Path(const Path& path): _id(path.getId()), _dsd(path.getDSD()){
	_nodes = path.getNodes();
	_edges = path.getEdges();
	_dist = path.getDistribution();
}
Path::Path(const Path& path, const Node_id& n, const Edge_id& e, const Distribution& dist): _id(path.getId()), _dsd(path.getDSD()){
	_nodes = path.getNodes();
	_nodes.push_back(n);
	_edges = path.getEdges();
	_edges.push_back(e);
	_dist = path.getDistribution().convolute(dist);
}
Path::Path(const std::deque<Node_id>& nodes, const std::deque<Edge_id>& edges, const Distribution& dist): _id(-1), _dsd(0){
	_nodes = nodes;
	_edges = edges;
	_dist = dist;
}

/*
 * Getters
 */
std::deque<Node_id> Path::getNodes() const { return _nodes; }
Node_id Path::getNode(const uint32_t& index) const { return _nodes[index]; }
uint32_t Path::getLength() const { return _nodes.size() ; }
Node_id Path::getFirstNode() const { return _nodes.front(); }
std::deque<Node_id>::const_iterator Path::first() const { return _nodes.begin(); }
Node_id Path::getLastNode() const { return _nodes.back(); }
std::deque<Node_id>::const_iterator Path::last() const { return _nodes.end(); }
std::deque<Edge_id> Path::getEdges() const { return _edges; }
Edge_id Path::getFirstEdge() const { return _edges.front(); }
Edge_id Path::getLastEdge() const { return _edges.back(); }
Distribution Path::getDistribution() const { return _dist; }
uint32_t Path::getShortestTime() const { return _dist.min(); }
uint32_t Path::getLargestTime() const { return _dist.max(); }
std::array<double,3> Path::getDistribT(const uint32_t& index) const{
	std::array<double,3> distribElement;
	distribElement[0] = _dist.getSupT(index);
	distribElement[1] = _dist.getPdfT(index);
	distribElement[2] = _dist.getCdfT(index);
	return distribElement;
}
double Path::getDSD() const { return _dsd; }
uint32_t Path::getId() const { return _id; }

/*
 * Setters
 */
void Path::setDSD(const int& degree){ _dsd = degree; }
void Path::setId(const uint32_t& index){ _id = index; }

/*
 * decrementDSD() method: decrement the degree of strong dominance
 */
void Path::decrementDSD(){ --_dsd; }

/*
 * incrementDSD() method: decrement the degree of strong dominance
 */
void Path::incrementDSD(){ ++_dsd; }

/*
 * append(const Path&) method: append current path with the one given as parameter (append node and edges sequences, and convolute distributions)
 * implicit hypothesis: the parameter is a reverse path, computed starting from destination => new path u->v starting from subpaths u->x and v->x
 */
Path Path::append(Path& suffix) {
	assert( suffix.getLastNode() == getLastNode() );
	std::deque<Node_id> nodes = _nodes;
	std::deque<Node_id> suffixNodes = suffix.getNodes();
	if( suffixNodes.size() == 1 ){
		setDSD(0);
		return *this;
	}
	for( std::deque<Node_id>::reverse_iterator itn = suffixNodes.rbegin()+1 ; itn != suffixNodes.rend() ; ++itn ){
		nodes.push_back( *itn );
	}
	std::deque<Edge_id> edges = _edges;
	std::deque<Edge_id> suffixEdges = suffix.getEdges();
	for( std::deque<Edge_id>::reverse_iterator ite = suffixEdges.rbegin() ; ite != suffixEdges.rend() ; ++ite ){
		edges.push_back( *ite );
	}
	Distribution newDist = _dist.convolute( suffix.getDistribution() );
	Path appendpath = Path(nodes, edges, newDist);
	return appendpath;
}

/*
 * acyclic() method: check if current path contains any cycle, and return true if the path is acyclic, false otherwise
 */
bool Path::acyclic(){
//	TRACE("Does this path contains any cycle? ... " << *this);
    std::map<Node_id, int> nodecount;
    for (auto itn = _nodes.begin(); itn != _nodes.end(); ++itn){
//    	TRACE("Node N" << *itn << " was found " << nodecount[*itn] << " time(s)...");
    	if( nodecount[*itn] > 0){
//    		TRACEF("CYCLE FOUND IN NODE SEQUENCE: N" << _nodes[0]);
//    		for(uint32_t n(1) ; n < _nodes.size() ; ++n){
//    			CONTINUE_STATUS( " -> N" << _nodes[n] );
//    		}
//    		CONTINUE_STATUS("\n");
    		return false;
    	}
    	nodecount[*itn]++;
    }
    return true;
}

/*
 * develop(const Graph* g) method: develop shortcut contained into the current path, so as to retrieve all original node that will be physically visited
 */
void Path::develop(const Graph* g){
	assert( _nodes.size() == _edges.size()+1);
	// The method does nothing if the path contains only one node
	if( _nodes.size() == 1 ){
		return;
	}
	std::deque<Node_id> newNodeSeq;
	newNodeSeq.push_back( getFirstNode() );
	std::deque<Edge_id> newEdgeSeq;
	std::deque<Node_id>::iterator itprevnode = _nodes.begin();
	std::deque<Node_id>::iterator itnode = itprevnode+1;
	std::deque<Edge_id>::iterator itedge = _edges.begin();
	while( itnode != _nodes.end() ){
//		TRACE("Current tuple: N" << *itprevnode << "-[E" << *itedge << "]->N" << *itnode);
		std::stack< std::tuple<Node_id,Edge_id,Node_id> > lifostack;
		lifostack.push( std::tuple<Node_id,Edge_id,Node_id>(*itprevnode , *itedge , *itnode) );
		while( !lifostack.empty() ){
			Node_id u = std::get<0>( lifostack.top() );
			Edge_id e = std::get<1>( lifostack.top() );
			Node_id v = std::get<2>( lifostack.top() );
			lifostack.pop();
			Node_id midnod(INVALID_NODE_ID);
			if( g->getLevel(u) < g->getLevel(v) ){
//				TRACE("Current stack item: N" << u << "(L" << g->getLevel(u) << ")-[fwE" << e << "]->N" << v << "(L" << g->getLevel(v) << ")" );
				midnod = g->getFwEdge(e).getMiddleNode();
			}
			else{
//				TRACE("Current stack item: N" << u << "(L" << g->getLevel(u) << ")-[bwE" << e << "]->N" << v << "(L" << g->getLevel(v) << ")" );
				midnod = g->getBwEdge(e).getMiddleNode();
			}
			if ( midnod == INVALID_NODE_ID ){
//				TRACE("Original edge => end of the search for this path section");
				newEdgeSeq.push_back(e);
				newNodeSeq.push_back(v);
				continue;
			}
			Edge_id incomingedge = INVALID_EDGE_ID, outcomingedge = INVALID_EDGE_ID;
			for(Edge_id edgein = g->getNodeBeginBW(midnod) ; edgein < g->getNodeEndBW(midnod) ; ++edgein ){
//				TRACE("INCOMING EDGE E" << edgein << ": N" << g->getBwEdge(edgein).getOrigin() << "->N" << g->getBwEdge(edgein).getDestination() );
				if( g->getBwEdge(edgein).getOrigin() == u){
					incomingedge = edgein;
					break;
				}
			}
			for(Edge_id edgeout = g->getNodeBeginFW(midnod) ; edgeout < g->getNodeEndFW(midnod) ; ++edgeout ){
//				TRACE("OUTCOMING EDGE E" << edgeout << ": N" << g->getFwEdge(edgeout).getOrigin() << "->N" << g->getFwEdge(edgeout).getDestination() );
				if( g->getFwEdge(edgeout).getDestination() == v){
					outcomingedge = edgeout;
					break;
				}
			}
//			TRACE("Add stack item: N" << midnod << "-[E" << outcomingedge << "]->N" << v);
			lifostack.push( std::make_tuple(midnod, outcomingedge, v) );
//			TRACE("Add stack item: N" << u << "-[E" << incomingedge << "]->N" << midnod);
			lifostack.push( std::make_tuple(u, incomingedge, midnod) );
		}
		++itprevnode;
		++itnode;
		++itedge;
	}
	_nodes.clear();
	_edges.clear();
	_nodes = newNodeSeq;
	_edges = newEdgeSeq;
//	TRACE("New path: " << *this);
}

/*
 * << operator: return an outstream version of the path (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const Path& path){
	os << "N" << path._nodes[0] ;
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
bool operator<(const Path& lhs, const Path& rhs){
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
