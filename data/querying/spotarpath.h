/*
 * spotarpath.h
 *
 *  Created on: 29 mars 2016
 *      Author: delhome
 */

#ifndef DATA_QUERYING_SPOTARPATH_H_
#define DATA_QUERYING_SPOTARPATH_H_

#include <deque>

#include "../../misc.h"
#include "../graph/distribution.h"
#include "../graph/graph.h"

class SpotarPath{
public:
	/*
	 * Constructors
	 */
	SpotarPath();
	SpotarPath(const uint32_t& id, const Node_id& n, const int& size, const uint32_t& delta);
	SpotarPath(const SpotarPath& path);
	SpotarPath(const SpotarPath& path, const Node_id& n, const Edge_id& e, const Distribution& dist);
	SpotarPath(const std::deque<Node_id>& nodes, const std::deque<Edge_id>& edges, const Distribution& dist);

	/*
	 * Getters
	 */
	uint32_t getId() const ;
	double getDSD() const ;
	uint32_t getSuffix() const ;
	std::deque<Node_id> getNodes() const ;
	Node_id getFirstNode() const;
	Node_id getLastNode() const;
	std::deque<Edge_id> getEdges() const ;
	Edge_id getFirstEdge() const;
	Edge_id getLastEdge() const;
	Distribution getDistribution() const ;
	uint32_t getShortestTime() const ;
	uint32_t getLargestTime() const ;
	std::array<double,3> getDistribT(const uint32_t& index) const;

	/*
	 * Setters
	 */
	void setId(const uint32_t& index);
	void setDSD(const int& degree);
	void setSuffix(const uint32_t& pathId);

	/*
	 * decrementDSD() method: decrement the degree of strong dominance
	 */
	void decrementDSD();

	/*
	 * incrementDSD() method: decrement the degree of strong dominance
	 */
	void incrementDSD();

	/*
	 * appendBW(const Node_id& , const Edge_id& , const Distribution& ) method: append a node to the current path (add it to the begin of the path)
	 */
	SpotarPath appendBW(const Node_id& node, const Edge_id& edge, const Distribution& dist);

	/*
	 * << operator: return an outstream version of the path (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, const SpotarPath& dist);

	/*
	 * < operator: compare two paths in terms of the visited node ids
	 * e.g. Path1:1->2->5 ; Path2:1->3->4 ==> Path1 < Path2 because of the second node (as the first node is common)
	 */
	friend bool operator<(const SpotarPath& lhs, const SpotarPath& rhs);

private:
	/*
	 * Attributes
	 */
	uint32_t _id;
	std::deque<Node_id> _nodes;
	std::deque<Edge_id> _edges;
	Distribution _dist;
	int _dsd; // Degree of strong dominance (in the context of path comparison, determinate on how many segment the path are the most reliable in terms of distribution cdf, see Nie, 2009)
	uint32_t _suffixId; // Subpath operator
};

#endif /* DATA_QUERYING_SPOTARPATH_H_ */
