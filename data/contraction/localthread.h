/*
 * localthread.h
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_LOCALTHREAD_H_
#define DATA_LOCALTHREAD_H_

#include "cachedwitness.h"
#include "../witnesssearch/witnesssearch.h"
#include "../graph/edge.h"

/*
 * Class LocalThread
 */
class LocalThread{
public:
	/*
	 * Constructors
	 */
	LocalThread(const LocalThread&) = delete;
	LocalThread& operator=(const LocalThread&) = delete;

	LocalThread(LocalThread&&) = default;
	LocalThread& operator=(LocalThread&&) = default;

	LocalThread(Graph* graph);

	/*
	 * Getters
	 */
	std::vector<Edge> getInsertedEdges();
	std::vector<WitnessCacheEntry> getCacheEntries();

	/*
	 * run() method: proceed to a local witness search
	 */
	uint8_t run(const Node_id& u_it, const Node_id& x_it, const Node_id& v_it, Distribution& dist_uxv);

	/*
	 *addEdge(Edge&&) method: add an edge to the edge list
	 */
	void addEdge(Edge&& newEdge);

	/*
	 * addCacheEntry(WitnessCacheEntry&&) method: add a witness in the local cache
	 */
	void addCacheEntry(WitnessCacheEntry&& wce);

	/*
	 * addCacheEntry(WitnessCacheEntry&&) method: add a witness in the local cache
	 */
	void addCacheEntry(const uint8_t& shortcut_status, const uint32_t& shortcut_complexity, const Node_id& u, const Node_id& x, const Node_id& v);

	/*
	 * clearEdgeList() method: clear the candidate edge list
	 */
	void clearEdgeList();

	/*
	 * clearCache() method: clear all the witness cache entries
	 */
	void clearCache();

	/*
	 * operator << : send some information about local witness search to an output stream (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, LocalThread& locthread);

protected:
	/*
	 * Attributes
	 */
	const Graph* _g;
	std::vector<Edge> _edgesToInsert;
	std::vector<WitnessCacheEntry> _witnessToCache;
	WitnessSearch _witnessSearch;
};



#endif /* DATA_LOCALTHREAD_H_ */
