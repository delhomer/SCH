/*
 * querysearchnode.h
 *
 *  Created on: 18 mars 2016
 *      Author: delhome
 */

#ifndef DATA_QUERYING_QUERYSEARCHNODE_H_
#define DATA_QUERYING_QUERYSEARCHNODE_H_

#include <boost/heap/pairing_heap.hpp>

#include "../witnesssearch/heapitem.h"
#include "../graph/distribution.h"
#include "routingpolicy.h"

/*
 * Class SearchNode: aiming at modeling a typical node during the search process
 * This node has an identifier, can be enqueued (or not), is the starting point of space-limited searches, is characterized by an arrival time and its distribution
 */
class QuerySearchNode{
public:
	/*
	 * Constructors
	 */
	QuerySearchNode();
	QuerySearchNode(const Node_id& nodeid, const uint32_t& nbpts, const uint32_t& delta);
	QuerySearchNode(const Node_id& nodeid, const bool& enqueuement, const uint32_t& nbpts, const uint32_t& delta);
	QuerySearchNode(const Node_id& nodeid, const bool& enqueuement, const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist);

	/*
	 * Getters
	 */
	boost::heap::pairing_heap<HeapElement>::handle_type getHandle() const;
	Node_id getNodeId() const;
	bool getEnqueuement() const;
	RoutingPolicy getPolicy() const;
	Distribution getDistribution() const;
	uint32_t getDistMin() const ;
	uint32_t getDistMax() const ;

	/*
	 * Setters
	 */
	void setNodeId(const Node_id& n);
	void setEnqueuement(const bool& enq);
	void setHandle(const boost::heap::pairing_heap<HeapElement>::handle_type& h);

	/*
	 * resetPolicy(const RoutingPolicy&, const Edge_id&, const Distribution&) method: reset the routing policy associated with current node
	 */
	void resetPolicy(const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist);

	/*
	 * addpaths(const RoutingPolicy&, const Edge_id&, const Distribution&) method: add new paths to the current node routing policy
	 * The candidate paths are built with the predecessor node policy and the edge linking both nodes
	 * return true if at least one candidate path has been successfully added to the routing policy
	 */
	bool addpaths(const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist);

	/*
	 * appendPolicy(const SearchNode&) method: append routing policy of current node with those of search node given as a parameter
	 */
	RoutingPolicy appendPolicy(const QuerySearchNode& alternode);

	/*
	 * infiniteDistrib() method: return true if current search node is characterized with an infinite distribution, false otherwise
	 */
	bool infiniteDistrib();

	/*
	 * aggregateDistrib(const Distribution&) method: proceed to
	 */
	void aggregateDistrib(const Distribution& distribution, const Edge_id& curEdge, const Node_id& newPredecessor);

	/*
	 * << operator: return an outstream version of the search node (printing purpose)
	 */
    friend std::ostream& operator<<(std::ostream& os, QuerySearchNode& sn);

private:
	/*
	 * Attributes
	 */
	boost::heap::pairing_heap<HeapElement>::handle_type _heapHandle;
	Node_id _nodeId;
	bool _isEnqueued : 8;
	RoutingPolicy _policy; // Routing policy associated to node _nodeId: how to reach starting node in the context with current node
};

#endif /* DATA_QUERYING_QUERYSEARCHNODE_H_ */
