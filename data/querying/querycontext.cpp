/*
 * querycontext.cpp
 *
 *  Created on: 18 mars 2016
 *      Author: delhome
 */

#include "querycontext.h"
#include <vector>

/*
 * Constructor
 */
QueryContext::QueryContext(): _specif(), _heap(), _searchNodes(){}
QueryContext::QueryContext(const Specif& spec): _specif(spec), _heap(), _searchNodes(){}

/*
 * Getters
 */
uint32_t QueryContext::getNbPts(){ return _specif.getNbPts(); }
double QueryContext::getDelta(){ return _specif.getDelta(); }
boost::heap::pairing_heap<HeapElement> QueryContext::getHeap(){ return _heap;	}
std::map<Node_id,QuerySearchNode> QueryContext::getSearchNodes(){ return _searchNodes;	}

/*
 * empty() method: return true if heap is empty, false otherwise
 */
bool QueryContext::empty() const{ return _heap.empty(); }

/*
 * contains(const Node_id&) method: return true if node_iterator is a valid index and if the corresponding Search Node is enqueued, false otherwise
 */
bool QueryContext::contains(const Node_id& nodeId) const {
	const auto it = _searchNodes.find( nodeId );
	if ( it == _searchNodes.end() ){
		return false;
	}
	return it->second.getEnqueuement();
}

/*
 * contains(const QuerySearchNode&) method: return true if search_node is enqueued, false otherwise
 */
bool QueryContext::contains(QuerySearchNode& searchNode) const { return searchNode.getEnqueuement(); }

/*
 * reached(const Node_id&) method: return true if node_iterator is a key of _hash_table
 */
bool QueryContext::reached(const Node_id& nodeId ) const { return _searchNodes.find(nodeId) != _searchNodes.end();	}

/*
 * getSearchNode(const Node_id&) method: return the SearchNode associated with node_iterator (const version)
 */
const QuerySearchNode& QueryContext::getSearchNode(const Node_id& nodeId) const{ return _searchNodes.find(nodeId)->second ; }

/*
 * getSearchNode(const Node_id&) method: return the SearchNode associated with node_iterator
 */
QuerySearchNode& QueryContext::getSearchNode(const Node_id& nodeId) { return _searchNodes.find(nodeId)->second ; }

/*
 * getMinId() method: return the id of the minimum element in the heap structure
 */
const Node_id QueryContext::getMinId() const{
	HeapElement he = _heap.top();
	return he.getNode();
}

/*
 * getMin() method: return the minimum element in the heap structure
 */
const QuerySearchNode QueryContext::getMin() const {
	return getSearchNode( getMinId() );
}

/*
 * getMinPriority() method: return the priority coefficient of the minimum element in the heap
 */
const double QueryContext::getMinPriority() const{
	if( _heap.empty() ){
		return std::numeric_limits<double>::max();
	}
	return _heap.top().getPriority();
}

/*
 * init(const NodeIterator&, const double&) method: insert a first element (node_iterator:priority) in the heap structure
 */
QuerySearchNode& QueryContext::init(const Node_id& nodeId, const double& priority ){
	assert( _heap.empty() );
	//	const Node_id newnodeid( _searchNodes.rbegin()->first + 1 );
	// Add the node in the priority queue
	const boost::heap::pairing_heap<HeapElement>::handle_type heap_handle = _heap.push( HeapElement(nodeId, priority) );
	// Add the node in the search node list
	_searchNodes[nodeId] = QuerySearchNode( nodeId , true , _specif.getNbPts()+1 , _specif.getDelta() );
	_searchNodes[nodeId].setHandle(heap_handle);
//	TRACE( "Insertion of node " << nodeId << " with priority " << priority);
	return _searchNodes[nodeId];
}

/*
 * insert(const NodeIterator&, const double&) method: insert a new element (node_iterator:priority) in the heap structure
 */
QuerySearchNode& QueryContext::insert(const Node_id& nodeId, const double& priority , RoutingPolicy& rp , Edge_id& e , Distribution& dist ){
	//	const Node_id newnodeid( _searchNodes.rbegin()->first + 1 );
	// Add the node in the priority queue
	const boost::heap::pairing_heap<HeapElement>::handle_type heap_handle = _heap.push( HeapElement(nodeId, priority) );
	// Add the node in the search node list
	_searchNodes[nodeId] = QuerySearchNode( nodeId , true , rp , e , dist );
	_searchNodes[nodeId].setHandle(heap_handle);
//	TRACE( "Insertion of node " << nodeId << " with priority " << priority);
	return _searchNodes[nodeId];
}

/*
 * insertAgain(SearchNode&, const double&) method: re-insert an element in the heap that was already inserted and deleted before
 */
void QueryContext::insertAgain(const Node_id& nodeid, const double& priority){
	const boost::heap::pairing_heap<HeapElement>::handle_type heap_handle = _heap.push( HeapElement(nodeid, priority) );
	getSearchNode(nodeid).setHandle(heap_handle);
	getSearchNode(nodeid).setEnqueuement(true);
	//	TRACE( "Re-insertion of node " << _search_nodes[search_node_id] << " with priority " << priority);
}

/*
 * decrease(SearchNode&, const double&) method: update an element of the heap (decrease the associated priority coefficient)
 */
void QueryContext::decrease(const Node_id& nodeid, const double& priority){
	const boost::heap::pairing_heap<HeapElement>::handle_type heap_handle = getSearchNode(nodeid).getHandle();
	_heap.increase(heap_handle, HeapElement(nodeid, priority));
	//	TRACE( "Key decreasing for node " << search_node << " -- new priority " << priority);
}

/*
 * managePolicy(RoutingPolicy& , Edge_id& , Distribution&, const bool&) method: update an element of the heap if it exists, add it otherwise
 */
bool QueryContext::managePolicy(const Node_id& nodeid, const double& candidateValue, RoutingPolicy& rp , Edge_id& e , Distribution& dist_uv ){
//	TRACE("Policy management: link node N" << nodeid << " with following policy: " << rp);
	QuerySearchNode& node = getSearchNode( nodeid );
	// If minimum possible value after edge u->v relaxation is larger than maximum value of node v distribution, the current candidate edge is skipped (deterministic dominance)
	if ( le( node.getDistMax() , candidateValue ) ){
//		TRACE("CANDIDATE IS DOMINATED BY EXISTING POLICY FRONTIER, DO NOT CONSIDER IT");
		return false;
	}
	// If candidate deterministically dominates current Pareto frontier: new best path, that dominates all other existing local reliable paths
	if( le(rp.getLargestTime() + dist_uv.max() , node.getDistMin() ) ){
//		TRACE("CANDIDATE DOMINATES CURRENT FRONTIER DETERMINISTICALLY");
		// The candidate path is better than all existing local-reliable path to node v, so reset the routing policy with a unique path (the candidate)
		// Insert candidate node into the priority queue with the shortest possible travel time of the policy
		getSearchNode(nodeid).resetPolicy(rp,e,dist_uv);
		return true;
	}
	// Here neither candidate or current frontier deterministically dominates the other, we have to compare the candidate with all local-reliable paths
	// Add the new paths into the routing policy, and if at least one path have been added, update the priority queue
//	TRACE("OVERLAPPING DISTRIBUTIONS (CANDIDATE AND FRONTIER) => TRY TO ADD NEW PATHS TO CURRENT BEST POLICY");
	if( getSearchNode(nodeid).addpaths(rp, e , dist_uv) ){
		//								TRACE("One path have been successfully added to the fw routing policy!");
		// Insert candidate node into the priority queue with the shortest possible travel time of the policy
		return true;
	}
	return false;
}

/*
 * update(SearchNode&, const double&) method: update an element of the heap if it exists, add it otherwise
 */
void QueryContext::update(const Node_id& nodeid, const double& priority ){
	if( !contains(nodeid) ){
		insertAgain( nodeid , priority );
	}
	else{
		decrease( nodeid , priority );
	}
}


/*
 * deleteMin() method: select the min element in the heap, remove it from this structure and return it
 */
QuerySearchNode& QueryContext::deleteMin(){
	QuerySearchNode& searchNode = _searchNodes[ _heap.top().getNode() ];
	//	TRACE( "Delete min element: " << _heap.top() << "(Corresponding search node: " << search_node << ")" );;
	_heap.pop();
	searchNode.setEnqueuement(false);
	return searchNode;
}

/*
 * clearPQ() method: set all search nodes out of the queue and clear it
 */
void QueryContext::clearPQ(){
	for ( auto heap_item : _heap ){
		_searchNodes[ heap_item.getNode() ].setEnqueuement(false);
	}
	_heap.clear();
}

/*
 * clearAll() method: clear all attributes of the current search context
 */
void QueryContext::clearAll(){
	_heap.clear();
	_searchNodes.clear();
}

/*
 * << operator: return an outstream version of the priority queue (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const QueryContext& qc){
	os << "********* PQ printing ************\n";
	os << qc._specif;
	os << "Heap elements:\n";
	for(auto elem: qc._heap){
		os << elem << "\n";
	}
	os << "Search nodes:\n";
	for(auto elem: qc._searchNodes){
		os << elem.second ;
	}
	os << "***********************************\n";
	return os;
}


