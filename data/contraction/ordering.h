/*
 * ordering.h
 *
 *  Created on: 19 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_CONTRACTION_ORDERING_H_
#define DATA_CONTRACTION_ORDERING_H_

#include "../graph/graph.h"
#include "localthread.h"
#include "cachedwitness.h"
#include "../../data_io/configreader.h"

/*
 * Class Ordering: describe the ordering process (hierarchy building)
 */
class Ordering{
public:
	/*
	 * Contructors
	 */
	Ordering();
	Ordering(Graph* g);

	/*
	 * Getters
	 */
	LocalThread& getLocThread(const uint32_t& index);
	double getContractionCost(const Node_id& u);
	std::vector<Node_id> getNodeIds() const;

	/*
	 * Setters
	 */
	void setConfig(double param_eq, double param_d, double param_oeq, double param_cq);

	/*
	 * simulateContraction(Node_id) method: initialize the hierarchy building by setting contraction cost of each node
	 * Node x contraction is simulated in order to evaluate the priority coefficient value at the beginning of the process
	 */
	void simulateContraction(const Node_id& n);

	/*
	 * shortcutNeeded(const Edge_id&, const Node_id&, const Edge_id&, const bool, Edge&) method:
	 */
	bool shortcutNeeded(const Edge_id& e_in, const Node_id& x, const Edge_id& e_out, const bool simulate, Edge& newEdge) ;

	/*
	 * contract() method: update the graph in order to fasten the querying algorithms (highlights important nodes and add shortcut edges where that is needed)
	 */
	void contract(const Edge_id& e_in, const Node_id x, const Edge_id& e_out, bool simulate = false);

	/*
	 * contraction(const Edge_id& , const Node_id , const Edge_id& , bool) method: determinate if a shortcut is needed to cover the deletion of provided node and edges, and save the shortcut in a set for further processing
	 * return true if a shortcut is needed, false otherwise
	 */
	bool contraction(const Edge_id& e_in, const Node_id x, const Edge_id& e_out, bool simulate);

	/*
	 * contractParallely() method: update the graph in order to fasten the querying algorithms (highlights important nodes and add shortcut edges where that is needed)
	 */
	void contractParallely();

	/*
	 * smallerCost(const Node_id&, const Node_id&) method: return true if node u has a smaller cost than node v, false otherwise
	 * ties are broken with respect to node ids (in order to guarantee contraction stability during different instances)
	 */
	bool smallerCost(const Node_id& u, const Node_id& v) const;

	/*
	 * localMinimum(const Node_id&, const int&) method:
	 */
	bool localMinimum(const Node_id& x, const int& hop_radius = 2) const;

	/*
	 * netContractionSet() method: choose nodes to be contracted next and move them in <code>_nodes</code>
	 * such that they occur after <code>_firstWorkingNode</code>. This includes
	 * setting up <code>_lastWorkingNode</code>, such that
	 *
	 * [_working_nodes_begin, _working_nodes_end)
	 *
	 * contains the nodes to be contracted next afterwards.
	 */
	void nextContractionSet();

	/*
	 * run(int) method: proceed to hierarchy construction and graph contraction
	 */
	std::vector<Edge> run(int nbThreads = -1);

	/*
	 * writeOrdering(std::string&) method: save node hierarchy and shortcuts into a text file
	 */
	void writeOrdering(const std::string& filename1, const std::string& filename2);

	/*
	 * << operator: send a short description of an Ordering instance into a output stream (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, Ordering& order);

private:
	/*
	 * Attributes
	 */
	Graph* _graph;
	WitnessCache _wcache;
	std::vector<LocalThread> _localThreads;
	Configs::Config _config;
	std::vector<Node_id> _nodeIds;
	std::vector<bool> _nextNodeToContract;
	std::vector<double> _contractionCost;
	std::vector<uint32_t> _nodeDepth; // Node depth in the hierarchy (for each node, indicates how many neighbors have been contracted before the node itself)
//	std::vector<uint32_t> _sortednodes; // Node level into the hierarchy (sorted by levels)
	uint32_t _firstWorkingNode;
	uint32_t _lastWorkingNode;
	std::vector<Edge> _newEdges;
};


#endif /* DATA_CONTRACTION_ORDERING_H_ */
