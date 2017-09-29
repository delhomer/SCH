/*
 * spotarpolicy.h
 *
 *  Created on: 29 mars 2016
 *      Author: delhome
 */

#ifndef DATA_QUERYING_SPOTARPOLICY_H_
#define DATA_QUERYING_SPOTARPOLICY_H_

#include <map>
#include <queue>
#include <set>

#include "../../misc.h"
#include "../graph/distribution.h"
#include "spotarpath.h"

class SpotarPolicy{
public:
	/*
	 * Constructors
	 */
	SpotarPolicy();
	SpotarPolicy(const Node_id& n, const int& size, const uint32_t& delta, const bool& empty=true);
	SpotarPolicy(const SpotarPath& path);

	/*
	 * Getters
	 */
	Node_id getNode() const;
	Distribution getFrontier() const;
	double getShortestTime() const;
	double getLargestTime() const;
	std::array<double,3> getFrontierT(const uint32_t& index) const;
	SpotarPath getFirstPath() const ;
	std::map<uint32_t,SpotarPath> getPaths() const ;
	std::map<uint32_t,SpotarPath> getLRPaths() const ;
	std::vector<uint32_t> getBestPathIds() const;
	uint32_t getBestPathId(const uint32_t& index) const;
	uint32_t getNbPaths() const;
	uint32_t getNbLRPaths() const;

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
	 * addPath(const SpotarPath&) method: add a new path to the routing policy
	 */
	void addPath(const SpotarPath& path);

	/*
	 * addlrpath(SpotarPath&) method: add the path in the candidate set if it is local-reliable and update this set accordingly by erasing obsolete paths
	 * return true if the path is really local-reliable, false otherwise
	 */
	bool addLRpath(SpotarPath& candidatepath);

	/*
	 * lrcheck(const SpotarPath&) method: add a candidate path in the candidate set
	 */
	bool lrcheck( SpotarPath& candidatePath );

	/*
	 * lrcheck_NieWu(const SpotarPath&) method: add a candidate path in the candidate set
	 * version of Nie & Wu article (2009), with possibility to keep path that are dominated by group of other LR-paths
	 */
	bool lrcheck_NieWu( SpotarPath& candidatePath );

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
	 * << operator: return an outstream version of the routing policy (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, const SpotarPolicy& dist);

private:
	/*
	 * Attributes
	 */
	Node_id _node;
	std::map<uint32_t,SpotarPath> _paths;
	std::map<uint32_t,SpotarPath> _LRpaths;
	Distribution _frontier;
	std::vector<uint32_t> _bestpaths;
};



#endif /* DATA_QUERYING_SPOTARPOLICY_H_ */
