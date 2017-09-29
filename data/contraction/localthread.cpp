/*
 * localthread.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "localthread.h"

/*
 * Constructor
 */
LocalThread::LocalThread(Graph* graph): _g(graph), _witnessSearch(graph){}

/*
 * Getters
 */
std::vector<Edge> LocalThread::getInsertedEdges(){ return _edgesToInsert; }
std::vector<WitnessCacheEntry> LocalThread::getCacheEntries(){ return _witnessToCache;}

/*
 * run() method: proceed to a local witness search
 */
uint8_t LocalThread::run(const Node_id& u_it, const Node_id& x_it, const Node_id& v_it, Distribution& dist_uxv){
	return _witnessSearch.run(u_it, x_it, v_it, dist_uxv);
}

/*
 *addEdge(Edge&&) method: add an edge to the edge list
 */
void LocalThread::addEdge(Edge&& newEdge){
//	TRACE("---> Inserted edge: " << newEdge);
	_edgesToInsert.push_back( newEdge );
}

/*
 * addCacheEntry(WitnessCacheEntry&&) method: add a witness in the local cache
 */
void LocalThread::addCacheEntry(WitnessCacheEntry&& wce){
	_witnessToCache.push_back( wce );
}

/*
 * addCacheEntry(WitnessCacheEntry&&) method: add a witness in the local cache
 */
void LocalThread::addCacheEntry(const uint8_t& shortcut_status, const uint32_t& shortcut_complexity, const Node_id& u, const Node_id& x, const Node_id& v){
	_witnessToCache.push_back( WitnessCacheEntry(shortcut_status,shortcut_complexity,u,x,v) );
}

/*
 * clearEdgeList() method: clear the candidate edge list
 */
void LocalThread::clearEdgeList(){
	_edgesToInsert.clear();
}

/*
 * clearCache() method: clear all the witness cache entries
 */
void LocalThread::clearCache(){
	_witnessToCache.clear();
}

/*
 * operator << : send some information about local witness search to an output stream (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, LocalThread& locthread){
	TRACE("Local Thread printing / graph located in memory at " << locthread._g);
	TRACE("Edges that have to be inserted in the graph as shortcuts (#e=" << locthread._edgesToInsert.size() << "):");
	for(auto& elem: locthread._edgesToInsert){
		os << elem << "\n";
	}
	os <<"\n";
	TRACE("Witnesses that have to be inserted in the cache (#w=" << locthread._witnessToCache.size() << "):");
	for(auto& elem: locthread._witnessToCache){
		os << elem << "\n";
	}
	return os;
}
