/*
 * cachedwitness.h
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_CACHEDWITNESS_H_
#define DATA_CACHEDWITNESS_H_

#include <vector>

#include "../witnesssearch/witnesscacheentry.h"

/*
 * Class WitnessCache: structure gathering all the cached witnesses (one vector of entries per nodes)
 */
class WitnessCache{
public:

	/*
	 * Constructor
	 */
	WitnessCache(size_t n_nodes);

	/*
	 * empty(const Node_id&) method: return true if no witness cache entry is stored for node x
	 */
	bool empty(const Node_id& x);

	/*
	 * lookup(const Node_id&, const Node_id&, const Node_id&) method: over all shortcuts created because of node x contraction, return the witness cache entry corresponding to shortcut u->v
	 */
	const WitnessCacheEntry lookup(const Node_id& u, const Node_id& x, const Node_id& v);

	/*
	 * contains(const Node_id&, const Node_id&, const Node_id&) method: return true if a shortcut u->v has been created after node x contraction, false otherwise
	 */
	bool contains(const Node_id& u, const Node_id& x, const Node_id& v);

	/*
	 * insert(const WitnessCacheEntry&) method: add a shortcut associated with middle node x
	 * This shortcut is described by cache_entry
	 */
	void insert(WitnessCacheEntry& cache_entry);

	/*
	 * remove(Node_id) method: clear cache entries corresponding to middle node x
	 */
	void remove(Node_id x);

	/*
	 * remove(const Node_id&, const Node_id&, const Node_id&) method: clear shortcut u->v associated with middle node x
	 */
	void remove(const Node_id& u, const Node_id& x, const Node_id& v);

	/*
	 * << operator: return an outstream version of the witness cache (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, WitnessCache& cache);

	static const WitnessCacheEntry INVALID_ENTRY;

private:
	/*
	 * Attributes
	 */
	std::vector<std::vector<WitnessCacheEntry>> _data;
};

#endif /* DATA_CACHEDWITNESS_H_ */
