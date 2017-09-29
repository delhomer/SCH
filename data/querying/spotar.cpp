/*
 * spotar.cpp
 *
 *  Created on: 29 mars 2016
 *      Author: delhome
 */

#include "spotar.h"

/*
 * Constructors
 */
Spotar::Spotar(): _graph(), _destination(){}
Spotar::Spotar(Graph* graph): _graph(graph), _destination(INVALID_NODE_ID) {}
Spotar::Spotar(Graph* graph, const Node_id& d): _graph(graph), _destination(d) {
	_policies[d] = SpotarPolicy(d , graph->getSpecif().getNbPts()+1 , graph->getSpecif().getDelta() , false );
}

/*
 * Getters
 */
uint32_t Spotar::getDestination() const{ return _destination ; }
SpotarPolicy Spotar::getPolicy(const Node_id& node) const { return _policies.find(node)->second; }

/*
 * Setters
 */
void Spotar::setDestination(const Node_id& d){
	_destination = d;
	_policies[d] = SpotarPolicy(d , _graph->getSpecif().getNbPts()+1 , _graph->getSpecif().getDelta() , false );
}

/*
 * reset() method: reset Spotar structur so as to compute a new route
 */
void Spotar::reset(){
	assert( _pathqueue.empty() );
	_policies.clear();
	_destination = INVALID_NODE_ID;
}

/*
 * run(const Node_id&) method: compute Spotar frontier to reach destination node d starting from any other node in the graph
 */
std::map<Node_id,SpotarPolicy> Spotar::run(){
	// Initialization: take the first (and only) path of destination node (from destination to itself) and insert it into the search queue
	assert( _policies[_destination].getNbLRPaths() == 1 );
	SpotarPath initialPath = _policies[_destination].getFirstPath();
	_pathqueue.push( initialPath );
//	TRACE("[Node N" << initialPath.getFirstNode() << "] Push following path into the search queue:\n" << initialPath);
//		uint32_t counter(0);
	// While there are paths into the search queue, continue the algorithm
	while(! _pathqueue.empty() ){//&& counter < 5000 ){
		// Consider the first path of the queue (and take it out the queue)
		SpotarPath curPath = _pathqueue.front();
//		TRACE("[Node N" << curPath.getFirstNode() << "] Pop following path from the search queue:\n" << curPath);
		_pathqueue.pop();
		// Let focus on origin node: consider existing policy at this node and extend it to backward adjacent nodes
		Node_id j = curPath.getFirstNode();
		Distribution curpathdist = curPath.getDistribution();
		for ( Edge_id e = _graph->getNodeBeginBW(j) ; e != _graph->getNodeEndBW(j) ; ++e ){
//			TRACE("BW Edge E" << e << " relaxation: " << _graph->getBwEdge(e) );
			Node_id i( _graph->getBwEdge(e).getOrigin() );
			// If node i is still on current path, do not evaluate the node, just continue the process
			if( cyclecheck(curPath,i) ){
				continue;
			}
			// Here we know that node i is a new node, build subsequent candidate path
			Distribution edgeDist = _graph->getBwEdge(e).getWeight();
			SpotarPath candidatePath = curPath.appendBW( i , e , edgeDist );
			// If i has not been reached yet, initialize the corresponding policy
			if( _policies.find( i ) == _policies.end() ){
//				TRACE("EMPTY ROUTING POLICY: node not yet visited!!");
				candidatePath.setId(1);
				candidatePath.setDSD( edgeDist.getSize() );
				candidatePath.setSuffix( curPath.getId() );
				_policies[i] = SpotarPolicy( candidatePath );
				_pathqueue.push( candidatePath );
//				TRACE("[Node N" << candidatePath.getFirstNode() << "] Push following path into the search queue:\n" << candidatePath);
			}
			// Otherwise, node i has still been relaxed, there is an existing policy with alternative paths
			else{
//				TRACE("NODE ALREADY VISITED, there is a routing policy with " << _policies[i].getNbPaths() << " paths and " << _policies[i].getNbLRPaths() << " LR-paths!");
				candidatePath.setId( _policies[i].getNbPaths() + 1 );
				// Check if candidate path is local-reliable: if it is, set its suffix, and add it into the search queue
				if( lrcheck(i,candidatePath) ){
					candidatePath.setSuffix( curPath.getId() );
					_pathqueue.push( candidatePath );
//					TRACE("[Node N" << candidatePath.getFirstNode() << "] Push following path into the search queue:\n" << candidatePath);
				}
				_policies[i].addPath( candidatePath );
			}
		}
//		MARK("End of iteration " << ++counter << " (queue size: " << _pathqueue.size() << ")");
//		CONTINUE_STATUS("\n");
	}

	for(auto& p: _policies){
//		MARK("Clean policy from node N" << p.first);
		p.second.clean( _graph->getSpecif() );
//		CONTINUE_STATUS( p.second );
	}

	return _policies;
}

/*
 * lrcheck(SpotarPath&) method: check if path given as a parameter is local-reliable, return true if it is, false otherwise
 */
bool Spotar::lrcheck(const Node_id& i, SpotarPath& candidatepath){
	return _policies[i].lrcheck(candidatepath);
}

/*
 * cyclecheck(SpotarPath&) method: return true if node i is still in path given as a parameter, false otherwise
 */
bool Spotar::cyclecheck(const SpotarPath& path, const Node_id& i){
	for(auto node: path.getNodes()){
		if( node == i ){
//			TRACE("Node N" << i << " is still on the path.");
			return true;
		}
	}
	return false;
}
