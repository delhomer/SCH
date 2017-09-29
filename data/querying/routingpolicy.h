/*
 * routingPolicy.h
 *
 *  Created on: 9 mars 2016
 *      Author: delhome
 */

#ifndef DATA_QUERYING_ROUTINGPOLICY_H_
#define DATA_QUERYING_ROUTINGPOLICY_H_

#include <map>
#include <set>

#include "../../misc.h"
#include "../graph/distribution.h"
#include "path.h"


class RoutingPolicy{
public:
	/*
	 * Constructors
	 */
	RoutingPolicy();
	RoutingPolicy(const Node_id& n, const int& size, const uint32_t& delta, const bool& empty=true);
	RoutingPolicy(const Node_id& n, const std::map<uint32_t,Path>& paths, const Distribution& dist, const std::vector<uint32_t>& bp);
	RoutingPolicy(const RoutingPolicy& rp, const Node_id& n, const Edge_id& e, const Distribution& dist);

	/*
	 * Getters
	 */
	Node_id getNode() const;
	Distribution getFrontier() const;
	double getShortestTime() const;
	double getLargestTime() const;
	std::array<double,3> getFrontierT(const uint32_t& index) const;
	std::map<uint32_t,Path> getPaths() const ;
	std::vector<uint32_t> getBestPathIds() const;
	uint32_t getBestPathId(const uint32_t& index) const;
	uint32_t getNbPaths() const;

	/*
	 * Setters
	 */
	void setNode(const Node_id& n);
	void setFrontier(const Distribution& dist);
	void setFrontierT(const uint32_t& index, const double& pdfValue, const double& cdfValue);
	void getBestPathIds(const std::vector<uint32_t>& bp);

	/*
	 * empty() method: return true if there is not any path is the policy (the distribution must be infinite in this case), false otherwise
	 */
	bool empty();

	/*
	 * clear() method: delete the saved paths and reset the frontier as an infinite distribution
	 */
	void clear();

	/*
	 * reset(const RoutingPolicy&, const Edge_id&, const Distribution&) method: reset the routing policy and fill it with a new path
	 */
	void reset(const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist);

	/*
	 * addPathClean(SpotarPath&) method: add the path in the candidate set if it is local-reliable and update this set accordingly by erasing obsolete paths
	 * return true if the path is really local-reliable, false otherwise
	 */
	bool addPathClean(Path& candidatepath, const uint32_t& candidateid);

	/*
	 * addPath(SpotarPath&) method: add the path in the candidate set if it is local-reliable
	 * return true if the path is really local-reliable, false otherwise
	 */
	bool addPath(Path& candidatepath, const uint32_t& candidateid);

	/*
	 * addPaths(const RoutingPolicy&, const Edge_id&, const Distribution&) method: try to add new paths to the path list, return true if at least one path have been added
	 * Starting from origin node routing policy, append current node to create new paths and compare them with existing paths of the policy
	 */
	bool addPaths(const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist);

	/*
	 * append() method: append the current routing policy with the one given as a parameter
	 */
	RoutingPolicy append(const RoutingPolicy& rp);

	/*
	 * merge(const RoutingPolicy&) method: merge the current routing policy with the candidate given as a parameter
	 * Concretely, add the candidate policy paths to the vector of current paths, and modify accordingly the '_frontier' and '_bestpaths' structures
	 */
	void merge(const RoutingPolicy& candidatePolicy);

	/*
	 * removeUselessPaths(std::set<uint32_t>&, const std::set<uint32_t>& ) method: remove paths identified by the first index set given as the first parameter
	 * the second parameter refers to IDs of valid paths; these paths are potential substitutes of removed paths
	 */
	void removeUselessPaths(std::set<uint32_t>& uselessPathIDs, const std::set<uint32_t>& usefulPathIDs);

	/*
	 * clean() method: erase path that are considered as local-reliable only for time budgets such that other optimal paths give comparable probability to be on-time
	 */
	void clean(const Specif& specif);

	/*
	 * developPaths(const Graph*) method: develop each local-reliable paths of the current routing policy
	 */
	void developPaths(const Graph* g);

	/*
	 * serialize(std::string& , std::string&) method: save the routing policy in two text files (one for routing policy distribution and one for path descriptions)
	 * format routing policy: [time budget t] <frontier.cdf[t]> <bestpathid[t]> <path1.cdf[t]> <path2.cdf[t]> ...
	 * format path nodes: [path p] <p.id> <p.node[1]> <p.node[2]> ... <p.node[N]> <NA> ... <NA>
	 * In this second file, complete the lines with as NA value as necessary (a line must contain as many node indices as the longer path
	 */
	void serialize(std::string& rpfilename, std::string& pnfilename);

	/*
	 * << operator: return an outstream version of the routing policy (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, const RoutingPolicy& dist);

private:
	/*
	 * Attributes
	 */
	Node_id _node;
	Distribution _frontier;
	std::map<uint32_t,Path> _paths;
	std::vector<uint32_t> _bestpaths;
};

#endif /* DATA_QUERYING_ROUTINGPOLICY_H_ */
