/*
 * witnesscacheentry.cpp
 *
 *  Created on: 18 févr. 2016
 *      Author: delhome
 */

#include "witnesscacheentry.h"
#include "witnesssearch.h"

/*
 * Constructors
 */
WitnessCacheEntry::WitnessCacheEntry(): _shortcutStatus(WitnessSearch::UNDECIDED), _shortcutComplexity(0), _u(INVALID_NODE_ID), _x(INVALID_NODE_ID), _v(INVALID_NODE_ID){}
WitnessCacheEntry::WitnessCacheEntry(const uint8_t& shortcut_status, const uint32_t& shortcut_complexity, const Node_id& u, const Node_id& x, const Node_id& v):
		_shortcutStatus(shortcut_status), _shortcutComplexity(shortcut_complexity), _u(u), _x(x), _v(v){}

/*
 * Getters
 */
uint8_t WitnessCacheEntry::getStatus(){ return _shortcutStatus;}
uint32_t WitnessCacheEntry::getComplexity(){ return _shortcutComplexity;}
Node_id WitnessCacheEntry::getOrigin(){ return _u;}
Node_id WitnessCacheEntry::getMiddleNode(){ return _x;}
Node_id WitnessCacheEntry::getDestination(){ return _v;}

/*
 * Setters
 */
void WitnessCacheEntry::setStatus(const uint8_t& s){ _shortcutStatus = s; }
void WitnessCacheEntry::setComplexity(const uint32_t& c){ _shortcutComplexity = c; }
void WitnessCacheEntry::setOrigin(const Node_id& u){ _u = u; }
void WitnessCacheEntry::setMiddleNode(const Node_id& x){ _x = x; }
void WitnessCacheEntry::setDestination(const Node_id& v){ _v = v; }

/*
 * << operator: return an outstream version of the witness cache entry (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, WitnessCacheEntry& entry){
	os << entry._u << "->" << entry._x << "->" << entry._v << " (status: " << unsigned(entry._shortcutStatus) << " complexity: " << entry._shortcutComplexity << ")";
	return os;
}
