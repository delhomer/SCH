/*
 * hierarchy.h
 *
 *  Created on: 8 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_HEAPITEM_H_
#define DATA_HEAPITEM_H_

#include <limits> // command numeric_limits::max()
#include <iostream>
#include <sstream>

#include "../../misc.h"

/*
 * Class HeapElement: model a basic heap element, ie a couple of value where the first element is the identifier and the second one is its priority coefficient
 */
class HeapElement{
public:
	/*
	 * Constructors
	 */
	HeapElement();
	HeapElement(const Node_id& search_node_id, const double& priority);

	/*
	 * Getters
	 */
	Node_id getNode() const;
	double getPriority() const;

	/*
	 * Setters
	 */
	void setNode(const Node_id& n);
	void setPriority(const double& d);

	/*
	 * toString() method: return the heap element in a string format
	 * format "{<id>:<priority>} "
	 */
	std::string toString();

	/*
	 * print() method: print the heap item on the console
	 * format "{<id>:<priority>} "
	 */
	void print();

    /*
     * << operator: return an outstream version of the interval (printing purpose)
	 * format "{<id>:<priority>} "
     */
    friend std::ostream& operator<<(std::ostream& os, const HeapElement& hitem);

	/*
	 * '<' operator: return true if left-hand-side element priority is smaller than right-hand-side element priority (heap element comparing purpose)
	 * As we are focused on min-heap, the relation is inverted
	 */
	friend inline bool operator< (const HeapElement& lhs, const HeapElement& rhs){ return rhs._priorCoef < lhs._priorCoef; }

private:
	/*
	 * Attributes
	 */
	Node_id _searchNodeId;
	double _priorCoef;
};

#endif /* DATA_HEAPITEM_H_ */
