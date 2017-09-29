/*
 * dyngraph.h
 *
 *  Created on: 28 avr. 2016
 *      Author: delhome
 */

#ifndef DATA_GRAPH_DYNGRAPH_H_
#define DATA_GRAPH_DYNGRAPH_H_

#include <chrono>
#include <omp.h> // Parallel computing commands

#include "dynedge.h"
#include "node.h"
#include "../../data_io/dyngraphreader.h"
#include "../../data_io/specifreader.h"

class DynGraph{
	/*
	 * Constructors
	 */
	DynGraph();
	DynGraph(DynGraphReader graphdata, const Specif& specif);

	/*
	 * Destructor
	 */
	~DynGraph();

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
	std::vector<DynEdge> getFwEdges() const;
	DynEdge getFwEdge(const Edge_id& e) const;
	std::vector<DynEdge> getBwEdges() const;
	DynEdge getBwEdge(const Edge_id& e) const;
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
    void setFwEdgeWeight(const Edge_id& edge_id, const uint32_t& t, Distribution& dist);
    void setBwEdgeWeight(const Edge_id& edge_id, const uint32_t& t, Distribution& dist);
    void setFwEdgeMidNod(const Edge_id& edge_id, const std::vector<Node>& midnods);
    void setBwEdgeMidNod(const Edge_id& edge_id, const std::vector<Node>& midnods);
    void setNodeBeginBW(const Node_id tgt, const Edge_id& e);
    void setNodeEndBW(const Node_id tgt, const Edge_id& e);
    void setNodeBeginFW(const Node_id src, const Edge_id& e);
    void setNodeEndFW(const Node_id src, const Edge_id& e);

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
	 * << operator: return an outstream version of the graph (printing purpose)
	 * format (line 1): <Specif>
	 * format (line 2->#nodes+1): {Ei0bw,...,Eikbw} -> Ni -> {Ei0fw,...,Eikfw}, where Eijbw is the edge linking nodes j and i (in a backward manner)
	 * format (line (#nodes+2)->#nodes+#edges+1) Ei: {(t_0,p_0,F_0) ... (t_k(i),p_k(i),F_k(i))}, where t, p and F are respectively the sets of support points, pmf and cdf // ie Ei: {<Distribution>}
	 */
    friend std::ostream& operator<<(std::ostream& os, DynGraph& graph);

private:
	/*
	 * Attributes
	 */
	Specif _specif;
	std::vector<Node> _nodes;
	std::vector<DynEdge> _fwedges;
	std::vector<DynEdge> _bwedges;
	std::vector<uint32_t> _levels; // Node level into the hierarchy (sorted by nodes)
	std::vector<Node_id> _sortednodes; // Node level into the hierarchy (sorted by levels)
};

#endif /* DATA_GRAPH_DYNGRAPH_H_ */
