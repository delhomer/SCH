/*
 * searchcontext.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "searchcontext.h"
#include <vector>

/*
 * Constructor
 */
SearchContext::SearchContext(): _heap(), _hash_table(), _searchNodes(), _specif(){}
SearchContext::SearchContext(const Specif& spec): _heap(), _hash_table(), _searchNodes(), _specif(spec){}

/*
 * Getters
 */
uint32_t SearchContext::getNbPts(){ return _specif.getNbPts(); }
double SearchContext::getDelta(){ return _specif.getDelta(); }
boost::heap::pairing_heap<HeapElement> SearchContext::getHeap(){ return _heap;	}
std::unordered_map<uint32_t,Node_id> SearchContext::getHashTable(){	return _hash_table;	}
std::vector<SearchNode> SearchContext::getSearchNodes(){ return _searchNodes;	}

/*
 * empty() method: return true if heap is empty, false otherwise
 */
bool SearchContext::empty() const{ return _heap.empty(); }

/*
 * contains(const Node_id&) method: return true if node_iterator is a valid index and if the corresponding Search Node is enqueued, false otherwise
 */
bool SearchContext::contains(const Node_id& node_iterator) const{
	const auto it = _hash_table.find(node_iterator);
	if ( it == _hash_table.end() ){
		return false;
	}
	SearchNode sn = _searchNodes[ it->second ];
	return sn.getEnqueuement();
}

/*
 * contains(const SearchNode&) method: return true if search_node is enqueued, false otherwise
 */
bool SearchContext::contains(SearchNode& search_node) const{ return search_node.getEnqueuement(); }

/*
 * reached(const Node_id&) method: return true if node_iterator is a key of _hash_table
 */
bool SearchContext::reached(const Node_id& node_iterator) const{ return _hash_table.find(node_iterator) != _hash_table.end();	}

/*
 * getSearchNode(const Node_id&) method: return the SearchNode associated with node_iterator (const version)
 */
const SearchNode& SearchContext::getSearchNode(const Node_id& node_iterator) const{ return _searchNodes[_hash_table.find(node_iterator)->second]; }

/*
 * getSearchNode(const Node_id&) method: return the SearchNode associated with node_iterator
 */
SearchNode& SearchContext::getSearchNode(const Node_id& node_iterator){ return _searchNodes[_hash_table.find(node_iterator)->second]; }

/*
 * getSearchNodeI(const Node_id&) method: return Search Node identified by index i
 */
SearchNode& SearchContext::getSearchNodeFromId(const Node_id& search_node_id){ return _searchNodes[search_node_id]; }

/*
 * getNodeId(const SearchNode&) method: starting from a Search Node, get the corresponding id in the _search_nodes vector
 */
Node_id SearchContext::getNodeId(const SearchNode& search_node) const{
	const Node_id search_node_id = (Node_id) std::distance( &(*(_searchNodes.begin())) , &search_node );
	return search_node_id;
}

/*
 * getMin() method: return the minimum element in the heap structure
 */
const SearchNode& SearchContext::getMin() const{
	HeapElement he = _heap.top();
	return _searchNodes[ he.getNode() ];
}

/*
 * getMinPriority() method: return the priority coefficient of the minimum element in the heap
 */
const double SearchContext::getMinPriority() const{
	if( _heap.empty() ){
		return std::numeric_limits<double>::max();
	}
	HeapElement he = _heap.top();
	return he.getPriority();
}

/*
 * insert(const NodeIterator&, const double&) method: insert a new element (node_iterator:priority) in the heap structure
 */
SearchNode& SearchContext::insert(const Node_id& node_iterator, const double& priority, const Node_id& predecessor_id){
	const Node_id search_node_id( _searchNodes.size() );
	_searchNodes.emplace_back();
	const boost::heap::pairing_heap<HeapElement>::handle_type heap_handle = _heap.push( HeapElement(search_node_id, priority) );
	_hash_table[node_iterator] = search_node_id;
	_searchNodes.back().setNodeId(node_iterator);
	_searchNodes.back().setPredId(predecessor_id);
	_searchNodes.back().setEnqueuement(true);
	_searchNodes.back().setHandle(heap_handle);
	//		TRACE("Specif data: " << _specif.getNbPts() << " support points shifted " << _specif.getDelta() << " by time units");
	_searchNodes.back().setDistribution( Distribution(_specif.getNbPts()+1,_specif.getDelta()) );
	//		_search_nodes.back().getDistribution().print();
	//		TRACE("New search node distribution is infinite? " << _search_nodes.back().getDistribution().isInfinite() );
	//	TRACE( "Insertion of node " << _search_nodes.back() << " with priority " << priority);
	return _searchNodes.back();
}

/*
 * insertAgain(SearchNode&, const double&) method: re-insert an element in the heap that was already inserted and deleted before
 */
void SearchContext::insertAgain(SearchNode& search_node, const double& priority){
	const Node_id search_node_id = getNodeId(search_node);
	const boost::heap::pairing_heap<HeapElement>::handle_type heap_handle = _heap.push( HeapElement(search_node_id, priority) );
	search_node.setHandle(heap_handle);
	search_node.setEnqueuement(true);
	//	TRACE( "Re-insertion of node " << _search_nodes[search_node_id] << " with priority " << priority);
}

/*
 * decrease(SearchNode&, const double&) method: update an element of the heap (decrease the associated priority coefficient)
 */
void SearchContext::decrease(SearchNode& search_node, const double& priority){
	const Node_id search_node_id = getNodeId(search_node);
	const boost::heap::pairing_heap<HeapElement>::handle_type heap_handle = search_node.getHandle();
	_heap.increase(heap_handle, HeapElement(search_node_id, priority));
	//	TRACE( "Key decreasing for node " << search_node << " -- new priority " << priority);
}

/*
 * update(SearchNode&, const double&) method: update an element of the heap if it exists, add it otherwise
 */
void SearchContext::update(SearchNode& search_node, const double& priority){
	if( !contains(search_node) ){
		insertAgain( search_node , priority );
	}else{
		decrease( search_node , priority );
	}
}


/*
 * deleteMin() method: select the min element in the heap, remove it from this structure and return it
 */
SearchNode& SearchContext::deleteMin(){
	HeapElement he = _heap.top();
	SearchNode& search_node = _searchNodes[ he.getNode() ];
	//	TRACE( "Delete min element: " << he << "(Corresponding search node: " << search_node << ")" );;
	_heap.pop();
	search_node.setEnqueuement(false);
	return search_node;
}

/*
 * clearPQ() method: set all search nodes out of the queue and clear it
 */
void SearchContext::clearPQ(){
	for ( auto heap_item : _heap ){
		_searchNodes[ heap_item.getNode() ].setEnqueuement(false);
	}
	_heap.clear();
}

/*
 * clearAll() method: clear all attributes of the current search context
 */
void SearchContext::clearAll(){
	_heap.clear();
	_hash_table.clear();
	_searchNodes.clear();
}

/*
 * << operator: return an outstream version of the priority queue (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const SearchContext& sc){
	os << "********* PQ printing ************\n";
	os << sc._specif;
	os << "Heap elements:\n";
	for(auto elem: sc._heap){
		os << elem << "\n";
	}
	os << "Hash table:\n";
	for(auto elem: sc._hash_table){
		os << "[" << elem.first << "->" << elem.second << "] ";
	}
	os << "\nSearch nodes:\n";
	for(auto elem: sc._searchNodes){
		os << elem << "\n";
	}
	os << "***********************************\n";
	return os;
}


