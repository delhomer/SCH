/*
 * priorityqueue.h
 *
 *  Created on: 9 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_SEARCHCONTEXT_H_
#define DATA_SEARCHCONTEXT_H_

#include <boost/heap/pairing_heap.hpp>
#include <unordered_map>

#include "../../data_io/specifreader.h"
#include "searchnode.h"

class SearchContext{
public:

	/*
	 * Constructor
	 */
	SearchContext();
	SearchContext(const Specif& spec);

	/*
	 * Getters
	 */
	uint32_t getNbPts();
	double getDelta();
	boost::heap::pairing_heap<HeapElement> getHeap();
	std::unordered_map<uint32_t,Node_id> getHashTable();
	std::vector<SearchNode> getSearchNodes();

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
	bool contains(SearchNode& search_node) const;

	/*
	 * reached(const Node_id&) method: return true if node_iterator is a key of _hash_table
	 */
	bool reached(const Node_id& node_iterator) const;

	/*
	 * getSearchNode(const Node_id&) method: return the SearchNode associated with node_iterator (const version)
	 */
	const SearchNode& getSearchNode(const Node_id& node_iterator) const;

	/*
	 * getSearchNode(const Node_id&) method: return the SearchNode associated with node_iterator
	 */
	SearchNode& getSearchNode(const Node_id& node_iterator);

	/*
	 * getSearchNodeI(const Node_id&) method: return Search Node identified by index i
	 */
	SearchNode& getSearchNodeFromId(const Node_id& search_node_id);

	/*
	 * getNodeId(const SearchNode&) method: starting from a Search Node, get the corresponding id in the _search_nodes vector
	 */
	Node_id getNodeId(const SearchNode& search_node) const;
	/*
	 * getMin() method: return the minimum element in the heap structure
	 */
	const SearchNode& getMin() const;

	/*
	 * getMinPriority() method: return the priority coefficient of the minimum element in the heap
	 */
	const double getMinPriority() const;

	/*
	 * insert(const NodeIterator&, const double&) method: insert a new element (node_iterator:priority) in the heap structure
	 */
	SearchNode& insert(const Node_id& node_iterator, const double& priority, const Node_id& predecessor_id = INVALID_NODE_ID);

	/*
	 * insertAgain(SearchNode&, const double&) method: re-insert an element in the heap that was already inserted and deleted before
	 */
	void insertAgain(SearchNode& search_node, const double& priority);

	/*
	 * decrease(SearchNode&, const double&) method: update an element of the heap (decrease the associated priority coefficient)
	 */
	void decrease(SearchNode& search_node, const double& priority);

	/*
	 * update(SearchNode&, const double&) method: update an element of the heap if it exists, add it otherwise
	 */
	void update(SearchNode& search_node, const double& priority);

	/*
	 * deleteMin() method: select the min element in the heap, remove it from this structure and return it
	 */
	SearchNode& deleteMin();

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
    friend std::ostream& operator<<(std::ostream& os, const SearchContext& sc);

private:
	/*
	 * Attributes
	 */
	boost::heap::pairing_heap<HeapElement> _heap; // Priority queue (composed of a set of elements organized as a heap)
	std::unordered_map<uint32_t, Node_id> _hash_table; // Map between ids and search nodes
	std::vector<SearchNode> _searchNodes; // Nodes to consider during the search
	Specif _specif; // Instance specification
};

#endif /* DATA_SEARCHCONTEXT_H_ */
