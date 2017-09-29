/*
 * cachedwitness.cpp
 *
 *  Created on: 18 févr. 2016
 *      Author: delhome
 */

#include "cachedwitness.h"

/*
 * Constructor
 */
WitnessCache::WitnessCache(size_t n_nodes): _data(n_nodes){}

/*
 * empty(const Node_id&) method: return true if no witness cache entry is stored for node x
 */
bool WitnessCache::empty(const Node_id& x){	return _data[x].size() == 0; }

/*
 * lookup(const Node_id&, const Node_id&, const Node_id&) method: over all shortcuts created because of node x contraction, return the witness cache entry corresponding to shortcut u->v
 */
const WitnessCacheEntry WitnessCache::lookup(const Node_id& u, const Node_id& x, const Node_id& v){
	for ( auto& cache_entry : _data[x] ){
		if ( cache_entry.getOrigin() == u && cache_entry.getDestination() == v ){
			return cache_entry;
		}
	}
	return WitnessCacheEntry();
}

/*
 * contains(const Node_id&, const Node_id&, const Node_id&) method: return true if a shortcut u->v has been created after node x contraction, false otherwise
 */
bool WitnessCache::contains(const Node_id& u, const Node_id& x, const Node_id& v){
	WitnessCacheEntry wcentry = lookup(u, x, v);
	return wcentry.getMiddleNode() != INVALID_NODE_ID;
}

/*
 * insert(const WitnessCacheEntry&) method: add a shortcut associated with middle node x
 * This shortcut is described by cache_entry
 */
void WitnessCache::insert(WitnessCacheEntry& cache_entry){
	_data[ cache_entry.getMiddleNode() ].push_back(cache_entry);
}

/*
 * remove(Node_id) method: clear cache entries corresponding to middle node x
 */
void WitnessCache::remove(Node_id x){
	_data[x].clear();
	_data[x].shrink_to_fit();
}

/*
 * remove(const Node_id&, const Node_id&, const Node_id&) method: clear shortcut u->v associated with middle node x
 */
void WitnessCache::remove(const Node_id& u, const Node_id& x, const Node_id& v){
	// For all entries corresponding to middle node x
	for ( int i = 0 ; i < (int) _data[x].size() ; ++i ){
		// If the entry is such that u is the origin (or invalid) and v is the destination (or invalid), delete the entry (ie delete all the entries u->x->_ and _->x->v
		if( ( u == INVALID_NODE_ID || u == _data[x][i].getOrigin() ) && ( v == INVALID_NODE_ID || v == _data[x][i].getDestination() ) ){
//			TRACE("Witness cache removal: " << _data[x][i]);
			_data[x][i] = _data[x].back();
			_data[x].pop_back();
			--i;
		}
	}
}

/*
 * << operator: return an output stream version of the witness cache (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, WitnessCache& cache){
	std::cout << "Witness cache printing\n";
	std::vector<std::vector<WitnessCacheEntry>>::iterator itwcebegin = cache._data.begin();
	for(std::vector<std::vector<WitnessCacheEntry>>::iterator itwce = itwcebegin ; itwce < cache._data.end() ; ++itwce){
		os << "Node " << itwce - itwcebegin << ":\n";
		for(auto &elem: *itwce){
			os << elem << "\n";
		}
	}
	return os;
}
