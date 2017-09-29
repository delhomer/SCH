/*
 * witnesssearch.h
 *
 *  Created on: 9 févr. 2016
 *      Author: delhome
 */

#include "../graph/graph.h"

#ifndef DATA_WITNESSSEARCH_WITNESSSEARCH_H_
#define DATA_WITNESSSEARCH_WITNESSSEARCH_H_

#include "searchcontext.h"

class Graph;

class WitnessSearch{
public:
	static constexpr uint8_t UNDECIDED = 0;
	static constexpr uint8_t NECESSARY = 1;
	static constexpr uint8_t NOT_NECESSARY = 2;
	/*
	 * Constructors
	 */
	WitnessSearch();
	WitnessSearch(Graph* graph);

	/*
	 * Getters
	 */

	/*
	 * run() method: proceed to witness search
	 * phase 1: backward interval search to reduce the search space in the backward graph
	 * phase 2: sample search to avoid useless profile search
	 * phase 3: profile search on the resulting graph to determine if a shortcut is necessary
	 * OUTPUT: integer index describing the shortcut needs (undecided: 0, necessary: 1, not necessary: 2)
	 */
	uint8_t run(const Node_id& u_it, const Node_id& x_it, const Node_id& v_it, Distribution& dist_uxv);

protected:
	/*
	 * backwardIntervalSearch(const Node_id, const Node_id, const uint32_t&) method: compute final interval label linked to reverse path destination<-start
	 * the interval bounds are set as the minimum and maximum of a distribution, namely Q(0) and Q(1), where Q denotes the quantile function
	 */
	void backwardIntervalSearch(const Node_id destination, const Node_id deletedNode, const Node_id start, const uint32_t& nbHops = 8);

	/*
	 * expectedTimeSearch(const Node_id, const Node_id, const uint32_t&) method: compute the least expected time for path start->destination
	 * starting from the node label computed by backward interval search, this procedure investigates on the least expected time path between start and destination in the predecessor graph
	 */
	void expectedTimeSearch(const Node_id& start, const Node_id& destination, const uint32_t& nbHops = 8);

	/*
	 * profileSearch(const Node_id&, const Node_id&, const uint32_t&) method:
	 */
	void profileSearch(const Node_id& start, const Node_id& destination, const uint32_t& nbHops = 8);

private:
	/*
	 * Attributes
	 */
	Graph* _graph;
	SearchContext _pq;
};



#endif /* DATA_WITNESSSEARCH_WITNESSSEARCH_H_ */
