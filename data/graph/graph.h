/*
 * graph.h
 *
 *  Created on: 3 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_GRAPH_H_
#define DATA_GRAPH_H_

#include <chrono>
#include <omp.h> // Parallel computing commands

#include "edge.h"
#include "node.h"
#include "../../data_io/graphreader.h"
#include "../../data_io/specifreader.h"

class LocalThread; // Forward class declaration: Graph structures has a vector of LocalThread pointers as an attribute
class WitnessCache;

class Graph{
public:
	/*
	 * Constructors
	 */

	Graph();
	Graph(GraphReader graphdata, const Specif& specif);

//	Graph(const Graph&) = delete;
//	Graph& operator= (const Graph&) = delete;
//	Graph(Graph&& graph) = default;
//	Graph& operator= (Graph&& graph) = default;

	/*
	 * Destructor
	 */
	~Graph();

	/*
	 * Getters
	 */
	Specif getSpecif() const;
	uint32_t getNbNodes() const;
	uint32_t getNbEdges() const;
	uint32_t getNbPtss() const;
	uint32_t getDelta() const;
	std::vector<Node> getNodes() const;
	Node getNode(const Node_id& n) const;
	std::vector<Edge> getFwEdges() const;
	Edge getFwEdge(const Edge_id& e) const;
	std::vector<Edge> getBwEdges() const;
	Edge getBwEdge(const Edge_id& e) const;
	std::vector<Node_id> getSortedNodes() const;
	Node_id getSortedNode(const uint32_t& index) const;
	std::vector<uint32_t> getLevels() const;
	uint32_t getLevel(const uint32_t& index) const;
    Edge_id getNodeBeginBW(Node_id tgt) const;
    Edge_id getNodeEndBW(Node_id tgt) const;
    Edge_id getNodeBeginFW(Node_id src) const;
    Edge_id getNodeEndFW(Node_id src) const;

    /*
     * Setters
     */
    void setLevel(const Node_id& n, const uint32_t& l);
    void setFwEdgeWeight(const Edge_id& edge_id, Distribution& dist);
    void setBwEdgeWeight(const Edge_id& edge_id, Distribution& dist);
    void setFwEdgeMidNod(const Edge_id& edge_id, const std::vector<Node>& midnods);
    void setBwEdgeMidNod(const Edge_id& edge_id, const std::vector<Node>& midnods);
    void setNodeBeginBW(const Node_id tgt, const Edge_id& e);
    void setNodeEndBW(const Node_id tgt, const Edge_id& e);
    void setNodeBeginFW(const Node_id src, const Edge_id& e);
    void setNodeEndFW(const Node_id src, const Edge_id& e);
    void setHierarchy(const std::string& hierarchyFileName);

    /*
     * aggregateEdge(const Edge_id&, const Edge&) method: aggregate forward edge at given index with candidate edge
     */
    void updateEdgeInfo(const Edge_id& fwe, const Edge_id& bwe, const Edge& candidateEdge);

    /*
     * aggregateFwEdge(const Edge_id&, const Edge&) method: aggregate forward edge at given index with candidate edge
     */
    void aggregateFwEdge(const Edge_id& e, const Edge& candidateEdge);

    /*
     * aggregateBwEdge(const Edge_id&, const Edge&) method: aggregate backward edge at given index with candidate edge
     */
    void aggregateBwEdge(const Edge_id& e, const Edge& candidateEdge);

    /*
     * addShortcut(const Edge&) method: add a new shortcut edge to the graph
     */
    void addShortcut(Edge&& edge);

    /*
     * identifyInsertId(const std::vector<Edge>&, Node_id, Node_id) method: return id at which a forward edge can be inserted
     */
    Edge_id identifyFwInsertId(const Node_id& node);

    /*
     * identifyInsertId(const std::vector<Edge>&, Node_id, Node_id) method: return id at which a backward edge can be inserted
     */
    Edge_id identifyBwInsertId(const Node_id& node);

    /*
     * identifyFwEdge(const Node_id&, const Node_id&) method: if there is a forward edge between given source and destination, return its id (INVALID_EDGE_ID otherwise)
     */
    Edge_id identifyFwEdge(const Node_id& origin, const Node_id& destination);

    /*
     * identifyBwEdge(const Node_id&, const Node_id&) method: if there is a backward edge between given source and destination, return its id (INVALID_EDGE_ID otherwise)
     */
    Edge_id identifyBwEdge(const Node_id& origin, const Node_id& destination);

    /*
     * deleteNode(const Node_id&) method: delete a node from the graph, and all subsequent adjacent edges
     */
    void deleteNode(const Node_id& u);

    /*
     * reset() method: reset the graph (goes back to initial state before node hierarchy building)
     * reset the node levels (they are set as the node ids) and delete the shortcuts
     */
    void reset();

    /*
     * eraseShortcuts() method: erase the shortcuts (goes back to state before contraction, but preserve node hierarchy)
     */
    void eraseShortcuts();

	/*
	 * << operator: return an outstream version of the graph (printing purpose)
	 * format (line 1): <Specif>
	 * format (line 2->#nodes+1): {Ei0bw,...,Eikbw} -> Ni -> {Ei0fw,...,Eikfw}, where Eijbw is the edge linking nodes j and i (in a backward manner)
	 * format (line (#nodes+2)->#nodes+#edges+1) Ei: {(t_0,p_0,F_0) ... (t_k(i),p_k(i),F_k(i))}, where t, p and F are respectively the sets of support points, pmf and cdf // ie Ei: {<Distribution>}
	 */
    friend std::ostream& operator<<(std::ostream& os, Graph& graph);

private:

	/*
	 * Attributes
	 */
	Specif _specif;
	std::vector<Node> _nodes;
	std::vector<Edge> _fwedges;
	std::vector<Edge> _bwedges;
	std::vector<uint32_t> _levels; // Node level into the hierarchy (sorted by nodes)
	std::vector<Node_id> _sortednodes; // Node level into the hierarchy (sorted by levels)

	static constexpr double GROWTH_FACTOR = 1.5; // Used to dimension the dummy edge needs (some space is kept to potential future edges, as shortcuts are)
};

#endif /* DATA_GRAPH_H_ */
