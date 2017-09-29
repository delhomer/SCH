/*
 * heapitem.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "heapitem.h"

/*
 * Constructors
 */
HeapElement::HeapElement(): _searchNodeId( INVALID_NODE_ID ), _priorCoef( std::numeric_limits<uint32_t>::max() ){}
HeapElement::HeapElement(const Node_id& search_node_id, const double& priority): _searchNodeId( search_node_id ), _priorCoef( priority ){}

/*
 * Getters
 */
Node_id HeapElement::getNode() const { return _searchNodeId; }
double HeapElement::getPriority() const { return _priorCoef; }

/*
 * Setters
 */
void HeapElement::setNode(const Node_id& n){ _searchNodeId = n; }
void HeapElement::setPriority(const double& d){ _priorCoef = d; }

/*
 * << operator: return an outstream version of the interval (printing purpose)
 * format "{<id>:<priority>} "
 */
std::ostream& operator<<(std::ostream& os, const HeapElement& hitem){
	os << "{" << hitem._searchNodeId << ":" << hitem._priorCoef << "} ";
	return os;
}
