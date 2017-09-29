/*
 * witnesscacheentry.h
 *
 *  Created on: 18 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_CONTRACTION_WITNESSCACHEENTRY_H_
#define DATA_CONTRACTION_WITNESSCACHEENTRY_H_

#include <iostream>

#include "../../misc.h"

/*
 * Class WitnessCacheEntry: structure aiming at describing a shortcut between u and v, when x is contracted (is it needed? if yes, with which complexity?)
 */
class WitnessCacheEntry{
public:
	/*
	 * Constructors
	 */
	WitnessCacheEntry();
	WitnessCacheEntry(const uint8_t& shortcut_status, const uint32_t& shortcut_complexity, const Node_id& u, const Node_id& x, const Node_id& v);

	/*
	 * Getters
	 */
	uint8_t getStatus();
	uint32_t getComplexity();
	Node_id getOrigin();
	Node_id getMiddleNode();
	Node_id getDestination();

	/*
	 * Setters
	 */
	void setStatus(const uint8_t& s);
	void setComplexity(const uint32_t& c);
	void setOrigin(const Node_id& u);
	void setMiddleNode(const Node_id& x);
	void setDestination(const Node_id& v);

	/*
	 * << operator: return an output stream version of the witness cache entry (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, WitnessCacheEntry& entry);

private:
	/*
	 * Attributes
	 */
	uint8_t _shortcutStatus : 2;
	uint32_t _shortcutComplexity : 30;
	Node_id _u;
	Node_id _x;
	Node_id _v;
};



#endif /* DATA_CONTRACTION_WITNESSCACHEENTRY_H_ */
