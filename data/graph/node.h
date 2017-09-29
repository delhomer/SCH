/*
 * node.h
 *
 *  Created on: 3 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_NODE_H_
#define DATA_NODE_H_

#include <iostream>

#include "../../misc.h"

class Node{
public:
	/*
	 * Constructors
	 */
	Node();

	/*
	 * Getters
	 */
	Edge_id getBeginFW() const;
	Edge_id getEndFW() const;
	Edge_id getBeginBW() const;
	Edge_id getEndBW() const;

	/*
	 * Setters
	 */
	void setBeginFW(const Edge_id& n);
	void setEndFW(const Edge_id& n);
	void setBeginBW(const Edge_id& n);
	void setEndBW(const Edge_id& n);

	void decreaseBeginFW();
	void decreaseEndFW();
	void decreaseBeginBW();
	void decreaseEndBW();
	void increaseBeginFW();
	void increaseEndFW();
	void increaseBeginBW();
	void increaseEndBW();

	/*
	 * print() method: print the node on the console
	 * format: [FW] <<begin>-<end>> [BW] <<begin>-<end>>
	 */
	void print();

    friend std::ostream& operator<<(std::ostream& os, const Node& node);

private:
	/*
	 * Attributes
	 */
	Edge_id _fwbegin;
	Edge_id _fwend;
	Edge_id _bwbegin;
	Edge_id _bwend;

};

#endif /* DATA_NODE_H_ */
