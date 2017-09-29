/*
 * node.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "node.h"

/*
 * Constructors
 */
Node::Node(): _fwbegin(INVALID_EDGE_ID), _fwend(INVALID_EDGE_ID), _bwbegin(INVALID_EDGE_ID), _bwend(INVALID_EDGE_ID){}

/*
 * Getters
 */
Edge_id Node::getBeginFW() const{ return _fwbegin; }
Edge_id Node::getEndFW() const{ return _fwend; }
Edge_id Node::getBeginBW() const{ return _bwbegin; }
Edge_id Node::getEndBW() const{ return _bwend; }

/*
 * Setters
 */
void Node::setBeginFW(const Edge_id& n){ _fwbegin = n; }
void Node::setEndFW(const Edge_id& n){ _fwend = n; }
void Node::setBeginBW(const Edge_id& n){ _bwbegin = n; }
void Node::setEndBW(const Edge_id& n){ _bwend = n; }

void Node::decreaseBeginFW(){ --_fwbegin; }
void Node::decreaseEndFW(){ --_fwend; }
void Node::decreaseBeginBW(){ --_bwbegin; }
void Node::decreaseEndBW(){ --_bwend; }
void Node::increaseBeginFW(){ ++_fwbegin; }
void Node::increaseEndFW(){ ++_fwend; }
void Node::increaseBeginBW(){ ++_bwbegin; }
void Node::increaseEndBW(){ ++_bwend; }

/*
 * << operator: return an outstream version of the node (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const Node& node){
	os << "[FW] <" << node._fwbegin << "-" << node._fwend << "> [BW] <" << node._bwbegin << "-" << node._bwend << ">\n";
    return os;
}
