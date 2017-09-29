/*
 * querysearchnode.cpp
 *
 *  Created on: 18 mars 2016
 *      Author: delhome
 */

#include "querysearchnode.h"

/*
 * Constructors
 */
QuerySearchNode::QuerySearchNode(): _heapHandle(), _nodeId(INVALID_NODE_ID), _isEnqueued(false) {}
QuerySearchNode::QuerySearchNode(const Node_id& nodeid, const uint32_t& nbpts, const uint32_t& delta): _heapHandle(), _nodeId(INVALID_NODE_ID), _isEnqueued(false), _policy(nodeid,nbpts,delta,true) {}
QuerySearchNode::QuerySearchNode(const Node_id& nodeid, const bool& enqueuement, const uint32_t& nbpts, const uint32_t& delta): _heapHandle(), _nodeId(nodeid), _isEnqueued(enqueuement), _policy(nodeid,nbpts,delta,false) {}
QuerySearchNode::QuerySearchNode(const Node_id& nodeid, const bool& enqueuement, const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist): _heapHandle(), _nodeId(nodeid), _isEnqueued(enqueuement), _policy(rp,nodeid,e,dist) {}

/*
 * Getters
 */
boost::heap::pairing_heap<HeapElement>::handle_type QuerySearchNode::getHandle() const { return _heapHandle; }
Node_id QuerySearchNode::getNodeId() const { return _nodeId; }
bool QuerySearchNode::getEnqueuement() const { return _isEnqueued; }
RoutingPolicy QuerySearchNode::getPolicy() const { return _policy; }
Distribution QuerySearchNode::getDistribution() const { return _policy.getFrontier(); }
uint32_t QuerySearchNode::getDistMin() const { return _policy.getShortestTime() ;}
uint32_t QuerySearchNode::getDistMax() const {return _policy.getLargestTime() ;}

/*
 * Setters
 */
void QuerySearchNode::setHandle(const boost::heap::pairing_heap<HeapElement>::handle_type& h){ _heapHandle = h; }
void QuerySearchNode::setNodeId(const Node_id& n){ _nodeId = n; }
void QuerySearchNode::setEnqueuement(const bool& enq){ _isEnqueued = enq; }

/*
 * resetPolicy(const RoutingPolicy&, const Edge_id&, const Distribution&) method: reset the routing policy associated with current node
 */
void QuerySearchNode::resetPolicy(const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist){
	_policy.reset(rp,e,dist);
}

/*
 * addpaths(const RoutingPolicy&, const Edge_id&, const Distribution&) method: add new paths to the current node routing policy
 * The candidate paths are built with the predecessor node policy and the edge linking both nodes
 * return true if at least one candidate path has been successfully added to the routing policy
 */
bool QuerySearchNode::addpaths(const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist){
	return _policy.addPaths(rp,e,dist);
}

/*
 * appendPolicy(const SearchNode&) method: append routing policy of current node with those of search node given as a parameter
 */
RoutingPolicy QuerySearchNode::appendPolicy(const QuerySearchNode& alternode){
	return _policy.append( alternode.getPolicy() );
}

/*
 * << operator: return an outstream version of the search node (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, QuerySearchNode& sn){
	os << "Node N" << sn._nodeId << (sn._isEnqueued?" (Q) ":" (NQ) ") << sn._policy;
	return os;
}
