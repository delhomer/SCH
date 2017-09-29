/*
 * path.h
 *
 *  Created on: 2 mars 2016
 *      Author: delhome
 */

#ifndef DATA_QUERYING_PATH_H_
#define DATA_QUERYING_PATH_H_

#include <deque>

#include "../../misc.h"
#include "../graph/distribution.h"
#include "../graph/graph.h"

class Path{
public:
	/*
	 * Constructors
	 */
	Path();
	Path(const uint32_t& id, const Node_id& n, const int& size, const uint32_t& delta);
	Path(const Path& path);
	Path(const Path& path, const Node_id& n, const Edge_id& e, const Distribution& dist);
	Path(const std::deque<Node_id>& nodes, const std::deque<Edge_id>& edges, const Distribution& dist);

	/*
	 * Getters
	 */
	std::deque<Node_id> getNodes() const ;
	Node_id getNode(const uint32_t& index) const ;
	uint32_t getLength() const ;
	Node_id getFirstNode() const;
	std::deque<Node_id>::const_iterator first() const;
	Node_id getLastNode() const;
	std::deque<Node_id>::const_iterator last() const;
	std::deque<Edge_id> getEdges() const ;
	Edge_id getFirstEdge() const;
	Edge_id getLastEdge() const;
	Distribution getDistribution() const ;
	uint32_t getShortestTime() const ;
	uint32_t getLargestTime() const ;
	std::array<double,3> getDistribT(const uint32_t& index) const;
	uint32_t getId() const ;
	double getDSD() const ;

	/*
	 * Setters
	 */
	void setId(const uint32_t& index);
	void setDSD(const int& degree);

	/*
	 * decrementDSD() method: decrement the degree of strong dominance
	 */
	void decrementDSD();

	/*
	 * incrementDSD() method: decrement the degree of strong dominance
	 */
	void incrementDSD();

	/*
	 * append(const Path&) method: append current path with the one given as parameter (append node and edges sequences, and convolute distributions)
	 * implicit hypothesis: the parameter is a reverse path, computed starting from destination => new path u->v starting from subpaths u->x and v->x
	 */
	Path append(Path& suffix);

	/*
	 * acyclic() method: check if current path contains any cycle, and return true if the path is acyclic, false otherwise
	 */
	bool acyclic();

	/*
	 * develop(const Graph* g) method: develop shortcut contained into the current path, so as to retrieve all original node that will be physically visited
	 */
	void develop(const Graph* g);

	/*
	 * << operator: return an outstream version of the path (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, const Path& dist);

	/*
	 * < operator: compare two paths in terms of the visited node ids
	 * e.g. Path1:1->2->5 ; Path2:1->3->4 ==> Path1 < Path2 because of the second node (as the first node is common)
	 */
	friend bool operator<(const Path& lhs, const Path& rhs);

private:
	/*
	 * Attributes
	 */
	uint32_t _id;
	std::deque<Node_id> _nodes;
	std::deque<Edge_id> _edges;
	Distribution _dist;
	int _dsd; // Degree of strong dominance (in the context of path comparison, determinate on how many segment the path are the most reliable in terms of distribution cdf, see Nie, 2009)
};



#endif /* DATA_QUERYING_PATH_H_ */
