/*
 * querycontext.h
 *
 *  Created on: 18 mars 2016
 *      Author: delhome
 */

#ifndef DATA_QUERYING_QUERYCONTEXT_H_
#define DATA_QUERYING_QUERYCONTEXT_H_

#include <boost/heap/pairing_heap.hpp>

#include <map>

#include "../witnesssearch/heapitem.h"
#include "../graph/distribution.h"
#include "querysearchnode.h"
#include "routingpolicy.h"

class QueryContext{
public:
	/*
	 * Constructor
	 */
	QueryContext();
	QueryContext(const Specif& spec);

	/*
	 * Getters
	 */
	uint32_t getNbPts();
	double getDelta();
	boost::heap::pairing_heap<HeapElement> getHeap();
	std::map<Node_id,QuerySearchNode> getSearchNodes();

	/*
	 * empty() method: return true if heap is empty, false otherwise
	 */
	bool empty() const;

	/*
	 * contains(const Node_id&) method: return true if node_iterator is a valid index and if the corresponding Search Node is enqueued, false otherwise
	 */
	bool contains(const Node_id& node_iterator) const;

	/*
	 * contains(const SearchNode&) method: return true if search_node is enqueued, false otherwise
	 */
	bool contains(QuerySearchNode& search_node) const;

	/*
	 * reached(const Node_id&) method: return true if node_iterator is a key of _hash_table
	 */
	bool reached(const Node_id& node_iterator) const;

	/*
	 * getSearchNode(const Node_id&) method: return the SearchNode associated with node_iterator (const version)
	 */
	const QuerySearchNode& getSearchNode(const Node_id& node_iterator) const;

	/*
	 * getSearchNode(const Node_id&) method: return the SearchNode associated with node_iterator
	 */
	QuerySearchNode& getSearchNode(const Node_id& node_iterator);

	/*
	 * getMinId() method: return the id of the minimum element in the heap structure
	 */
	const Node_id getMinId() const;

	/*
	 * getMin() method: return the minimum element in the heap structure
	 */
	const QuerySearchNode getMin() const;

	/*
	 * getMinPriority() method: return the priority coefficient of the minimum element in the heap
	 */
	const double getMinPriority() const;

	/*
	 * init(const NodeIterator&, const double&) method: insert the first element (node_iterator:priority) in the heap structure
	 */
	QuerySearchNode& init(const Node_id& node_iterator, const double& priority );

	/*
	 * insert(const NodeIterator&, const double&) method: insert a new element (node_iterator:priority) in the heap structure
	 * Build its routing policy starting from its predecessor routing policy
	 */
	QuerySearchNode& insert(const Node_id& node_iterator, const double& priority , RoutingPolicy& rp , Edge_id& e , Distribution& dist );

	/*
	 * insertAgain(SearchNode&, const double&) method: re-insert an element in the heap that was already inserted and deleted before
	 */
	void insertAgain(const Node_id& nodeid, const double& priority);

	/*
	 * decrease(SearchNode&, const double&) method: update an element of the heap (decrease the associated priority coefficient)
	 */
	void decrease(const Node_id& nodeid, const double& priority);

//	/*
//	 * update(SearchNode&, const double&, RoutingPolicy& , Edge_id& , Distribution&, const bool&) method: update an element of the heap if it exists, add it otherwise
//	 */
//	void update(const Node_id& nodeid, const double& priority , RoutingPolicy& rp , Edge_id& e , Distribution& dist_uv , const bool& reset);

	/*
	 * managePolicy(RoutingPolicy& , Edge_id& , Distribution&, const bool&) method: update an element of the heap if it exists, add it otherwise
	 */
	bool managePolicy(const Node_id& nodeid, const double& candidate, RoutingPolicy& rp , Edge_id& e , Distribution& dist_uv );

	/*
	 * update(SearchNode&, const double&) method: update an element of the heap if it exists, add it otherwise
	 */
	void update(const Node_id& nodeid, const double& priority );

	/*
	 * deleteMin() method: select the min element in the heap, remove it from this structure and return it
	 */
	QuerySearchNode& deleteMin();

	/*
	 * clearPQ() method: set all search nodes out of the queue and clear it
	 */
	void clearPQ();

	/*
	 * clearAll() method: clear all attributes of the current search context
	 */
	void clearAll();

    /*
     * << operator: return an outstream version of the priority queue (printing purpose)
     */
    friend std::ostream& operator<<(std::ostream& os, const QueryContext& sc);

private:
	/*
	 * Attributes
	 */
	Specif _specif; // Instance specification
	boost::heap::pairing_heap<HeapElement> _heap; // Priority queue (composed of a set of elements organized as a heap)
	std::map<Node_id,QuerySearchNode> _searchNodes; // Nodes to consider during the search
};



#endif /* DATA_QUERYING_QUERYCONTEXT_H_ */
