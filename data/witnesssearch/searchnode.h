/*
 * priorityqueueelement.h
 *
 *  Created on: 9 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_SEARCHNODE_H_
#define DATA_SEARCHNODE_H_

#include <boost/heap/pairing_heap.hpp>

#include "heapitem.h"
#include "interval.h"
#include "../graph/distribution.h"

/*
 * Class SearchNode: aiming at modeling a typical node during the search process
 * This node has an identifier, can be enqueued (or not), is the starting point of space-limited searches, is characterized by an arrival time and its distribution
 */
class SearchNode{
public:
	/*
	 * Structure Predecessor: model a predecessor relationship (one node and its backward incoming edge)
	 */
	struct Predecessor{
		Edge_id _bw_edge_it;
		Node_id _search_node_id;
		Predecessor(): _bw_edge_it(INVALID_EDGE_ID), _search_node_id( INVALID_NODE_ID ){}
		Predecessor(const Edge_id& e, const Node_id& id): _bw_edge_it(e), _search_node_id(id){}
	};

	/*
	 * Constructors
	 */
//	SearchNode(const SearchNode&) = delete;
//	SearchNode& operator=(const SearchNode&) = delete;
//	SearchNode(SearchNode&&) = default;
//	SearchNode& operator=(SearchNode&&) = default;
	SearchNode();
	SearchNode(const uint32_t& size);

	/*
	 * Getters
	 */
	boost::heap::pairing_heap<HeapElement>::handle_type getHandle();
	Node_id getNodeId();
	Node_id getPredId();
	bool getEnqueuement();
	uint8_t getSampleHop();
	uint8_t getIntervalHop();
	uint8_t getProfileHop();
	Interval getInterval();
	Distribution& getDistribution();
	uint32_t getDistMin();
	uint32_t getDistMax();
	double getExpectedTime();
	Predecessor getPredecessorLB();
	Predecessor getPredecessorUB();
	Predecessor getPredecessorT(const uint32_t& index);
	std::pair<Predecessor,Predecessor> getBoundingPredecessors();
	std::vector<Predecessor> getPredecessors();

	/*
	 * Setters
	 */
	void setNodeId(const Node_id& n);
	void setPredId(const Node_id& n);
	void setEnqueuement(const bool& enq);
	void setHandle(const boost::heap::pairing_heap<HeapElement>::handle_type& h);
	void setInterval(const double& lb, const double& ub);
	void setDistribution(const Distribution& dist);
	void setSampleHop(const uint8_t& h);
	void setIntervalHop(const uint8_t& h);
	void setProfileHop(const uint8_t& h);
	void setExpectedTime(const double& arr);
	void setPredecessorLB(const Edge_id& elb, const Node_id& nlb);
	void setPredecessorUB(const Edge_id& eub, const Node_id& nub);
	void setPredecessorT(const uint32_t& index, const Edge_id& eub, const Node_id& nub);
	void setBoundingPredecessors(const Edge_id& elb, const Node_id& nlb, const Edge_id& eub, const Node_id& nub);
	void setPredecessors(const uint32_t& maxIndex, const Edge_id& e, const Node_id& n);

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
    friend std::ostream& operator<<(std::ostream& os, SearchNode& sn);

private:
	/*
	 * Attributes
	 */
	boost::heap::pairing_heap<HeapElement>::handle_type _heapHandle;
	Node_id _nodeId;
	Node_id _predecessorId; // Last visited node during the search
	bool _isEnqueued : 8;
	uint8_t _sampleHops;
	uint8_t _intervalHops;
	uint8_t _profileHops;
	Interval _inter;
	double _expectedTime;
	Distribution _dist;
	std::pair<Predecessor,Predecessor> _preds; // Best predecessors (resp. for lower and upper bounds)
	std::vector<Predecessor> _predecessors;
};

#endif /* DATA_SEARCHNODE_H_ */
