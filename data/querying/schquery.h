/*
 * schquery.h
 *
 *  Created on: 1 mars 2016
 *      Author: delhome
 */

#ifndef DATA_QUERYING_SCHQUERY_H_
#define DATA_QUERYING_SCHQUERY_H_

#include <algorithm> // Command min, max
#include <array>
#include <limits>       // std::numeric_limits
#include <map>
#include <set>

#include "../../misc.h"
#include "../graph/graph.h"
#include "querycontext.h"
#include "routingpolicy.h"

class SCHQuery{
public:
	/*
	 * Constructors
	 */
	SCHQuery(Graph* graph);

	/*
	 * Getters
	 */
	bool getDirection();

	/*
	 * flip() method: set _forwardDirection as true if it is false, as false otherwise and return the new value
	 */
	bool flip();

	/*
	 * oneToOne() method: compute the shortest path between given origin and destination nodes
	 * Work into the ordered graph
	 */
	RoutingPolicy oneToOne(const Node_id& start, const Node_id& destination);

	/*
	 * bidirectionalIntervalSearch() method: compute a bidirectional interval search on the hierarchized graph
	 * Set the _corridor attribute, aiming at reducing the search space for a further profile search
	 * Give the least (respectively the worst) possible travel time via minimum (respectively maximum) travel time probabilities
	 */
	void bidirectionalIntervalSearch();

	/*
	 * fillCorridor() method: recover the node that are in the reduced corridor, after bidirectional interval search
	 */
	void fillCorridor();

	/*
	 * profileSearch() method: compute the stochastic shortest path by the way of a bidirectional profile search
	 * The algorithm can work on the whole hierarchized graph, or more efficiently, in the reduced corridor given by bidirectionalIntervalSearch() method
	 * Give the best travel time distribution to link start and destination nodes and associated predecessor graph (paths may be rebuilt subsequently)
	 */
	void bidirectionalProfileSearch();

	/*
	 * buildSolution(set<Node_id>) method: build the final routing policy to join the destination, by merging the routing policy using each search meeting nodes
	 */
	RoutingPolicy buildSolution(std::set<Node_id> meetingNodes);

	/*
	 * operator << : send to an output stream the SCHQuery structure
	 */
	friend std::ostream& operator<<(std::ostream& os, SCHQuery& schq);

private:
	/*
	 * Attributes
	 */
	Graph* _graph;
    Node_id _start;
    Node_id _destination;
    bool _forwardDirection;
    double _targettedProb;
    double _timeBudget;
    std::array<QueryContext,2> _priorityQueues;
    std::set<Node_id> _candidates;
    double _uppB;
};

#endif /* DATA_QUERYING_SCHQUERY_H_ */
