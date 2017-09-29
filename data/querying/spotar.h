/*
 * spotar.h
 *
 *  Created on: 29 mars 2016
 *      Author: delhome
 */

#ifndef DATA_QUERYING_SPOTAR_H_
#define DATA_QUERYING_SPOTAR_H_

#include "../../misc.h"
#include "../graph/graph.h"
#include "querycontext.h"
#include "routingpolicy.h"
#include "spotarpolicy.h"

class Spotar{
public:
	/*
	 * Constructors
	 */
	Spotar();
	Spotar(Graph* graph);
	Spotar(Graph* graph, const Node_id& d);

	/*
	 * Getters
	 */
	uint32_t getDestination() const;
	SpotarPolicy getPolicy(const Node_id& node) const;

	/*
	 * Setters
	 */
	void setDestination(const Node_id& d);

	/*
	 * reset() method: reset Spotar structur so as to compute a new route
	 */
	void reset();

	/*
	 * run(const Node_id&) method: compute Spotar frontier to reach destination node d
	 */
	std::map<Node_id,SpotarPolicy> run();

	/*
	 * lrcheck(SpotarPath&) method: check if path given as a parameter is local-reliable, return true if it is, false otherwise
	 */
	bool lrcheck(const Node_id& i, SpotarPath& candidatepath);

	/*
	 * cyclecheck(SpotarPath&) method: return true if node i is still in path given as a parameter, false otherwise
	 */
	bool cyclecheck(const SpotarPath& path, const Node_id& i);

private:
	/*
	 * Attributes
	 */
	Graph* _graph;
    Node_id _destination;
    std::map<Node_id,SpotarPolicy> _policies;
    std::queue<SpotarPath> _pathqueue;
};

#endif /* DATA_QUERYING_SPOTAR_H_ */
