/*
 * searchnode.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "searchnode.h"

/*
 * Constructors
 */
SearchNode::SearchNode(): _heapHandle(), _nodeId(INVALID_NODE_ID), _predecessorId(INVALID_NODE_ID), _isEnqueued(false), _sampleHops(0), _intervalHops(0), _profileHops(0),
_inter(), _expectedTime(std::numeric_limits<double>::max()), _dist(), _preds(), _predecessors(){}
SearchNode::SearchNode(const uint32_t& size): _heapHandle(), _nodeId(INVALID_NODE_ID), _predecessorId(INVALID_NODE_ID), _isEnqueued(false), _sampleHops(0), _intervalHops(0), _profileHops(0),
_inter(), _expectedTime(std::numeric_limits<double>::max()), _dist(size), _preds(), _predecessors(){}

/*
 * Getters
 */
boost::heap::pairing_heap<HeapElement>::handle_type SearchNode::getHandle(){ return _heapHandle; }
Node_id SearchNode::getNodeId(){ return _nodeId; }
Node_id SearchNode::getPredId(){ return _predecessorId; }
bool SearchNode::getEnqueuement(){ return _isEnqueued; }
uint8_t SearchNode::getSampleHop(){ return _sampleHops; }
uint8_t SearchNode::getIntervalHop(){ return _intervalHops; }
uint8_t SearchNode::getProfileHop(){ return _profileHops; }
Interval SearchNode::getInterval(){ return _inter; }
Distribution& SearchNode::getDistribution(){ return _dist; }
uint32_t SearchNode::getDistMin(){ return _dist.min(); }
uint32_t SearchNode::getDistMax(){ return _dist.max(); }
double SearchNode::getExpectedTime(){ return _expectedTime; }
SearchNode::Predecessor SearchNode::getPredecessorLB(){ return _preds.first; }
SearchNode::Predecessor SearchNode::getPredecessorUB(){ return _preds.second; }
SearchNode::Predecessor SearchNode::getPredecessorT(const uint32_t& index){ return _predecessors[index]; }
std::pair<SearchNode::Predecessor,SearchNode::Predecessor> SearchNode::getBoundingPredecessors(){ return _preds; }
std::vector<SearchNode::Predecessor> SearchNode::getPredecessors(){ return _predecessors; }

/*
 * Setters
 */
void SearchNode::setNodeId(const Node_id& n){ _nodeId = n; }
void SearchNode::setPredId(const Node_id& n){ _predecessorId = n; }
void SearchNode::setEnqueuement(const bool& enq){ _isEnqueued = enq; }
void SearchNode::setHandle(const boost::heap::pairing_heap<HeapElement>::handle_type& h){ _heapHandle = h; }
void SearchNode::setInterval(const double& lb, const double& ub){ _inter.setInterval(lb, ub); }
void SearchNode::setDistribution(const Distribution& dist){ _dist = dist; _predecessors.assign(_dist.getSize(),Predecessor()); }
void SearchNode::setSampleHop(const uint8_t& h){ _sampleHops = h; }
void SearchNode::setIntervalHop(const uint8_t& h){ _intervalHops = h; }
void SearchNode::setProfileHop(const uint8_t& h){ _profileHops = h; }
void SearchNode::setExpectedTime(const double& arr){ _expectedTime = arr; }
void SearchNode::setPredecessorLB(const Edge_id& elb, const Node_id& nlb){
	_preds.first._bw_edge_it = elb;
	_preds.first._search_node_id = nlb;
}
void SearchNode::setPredecessorUB(const Edge_id& eub, const Node_id& nub){
	_preds.second._bw_edge_it = eub;
	_preds.second._search_node_id = nub;
}
void SearchNode::setBoundingPredecessors(const Edge_id& elb, const Node_id& nlb, const Edge_id& eub, const Node_id& nub){
	setPredecessorLB(elb,nlb);
	setPredecessorUB(eub,nub);
}
void SearchNode::setPredecessorT(const uint32_t& index, const Edge_id& eub, const Node_id& nub){
	_predecessors[index]._bw_edge_it = eub;
	_predecessors[index]._search_node_id = nub;
}
void SearchNode::setPredecessors(const uint32_t& maxIndex, const Edge_id& e, const Node_id& n){
	for(uint32_t t(0) ; t < maxIndex ; ++t){
		setPredecessorT(t,e,n);
	}
}

/*
 * infiniteDistrib() method: return true if current search node is characterized with an infinite distribution, false otherwise
 */
bool SearchNode::infiniteDistrib(){ return _dist.isInfinite(); }

/*
 * aggregateDistrib(const Distribution&) method: proceed to
 */
void SearchNode::aggregateDistrib(const Distribution& distribution, const Edge_id& curEdge, const Node_id& newPredecessor){
	for(uint32_t t(0) ; t < distribution.getSize() ; ++t){
		if( lt( _dist.getCdfT(t) , distribution.getCdfT(t) ) ){
			_dist.setCdfT( t , distribution.getCdfT(t) );
			setPredecessorT(t, curEdge, newPredecessor );
		}
		if(t == 0){
			_dist.setPdfT( t , _dist.getCdfT(t) );
		}else{
			_dist.setPdfT( t , _dist.getCdfT(t)-_dist.getCdfT(t-1) );
		}
	}
}

/*
 * << operator: return an outstream version of the search node (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, SearchNode& sn){
	os << "Node N" << sn._nodeId << ", from N" << sn._predecessorId << (sn._isEnqueued?" (Q) ":" (NQ) ") << "Predecessors: [" << sn._preds.first._search_node_id << ";" << sn._preds.second._search_node_id << "] "
			<< "Interval: " << sn._inter << " E(t)=" << sn._expectedTime << " Distribution: " << sn._dist;
	return os;
}

