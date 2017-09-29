/*
 * dynedge.h
 *
 *  Created on: 28 avr. 2016
 *      Author: delhome
 */

#ifndef DATA_GRAPH_DYNEDGE_H_
#define DATA_GRAPH_DYNEDGE_H_

#include "distribution.h"

class DynEdge{
public:
	/*
	 * Constructors
	 */
	DynEdge();
	DynEdge(const bool& forward);
	DynEdge(const bool& fw, const Node_id& src, const Node_id& dest);
	DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, Distribution& dist);
	DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, std::vector<Distribution>& dist, const Node_id& midnod);
	DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, Distribution& dist, const uint32_t& complex, const uint32_t& origedge, const std::vector<Node_id>& midnod);
	DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, const Edge_id& e, Distribution& dist);
	DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, const Edge_id& e, std::vector<Distribution>& dist, const Node_id& midnod);
	DynEdge(const DynEdge& edge);

	/*
	 * Getters
	 */
	Node_id getOrigin() const;
	Node_id getDestination() const ;
    bool getBW() const;
    bool getFW() const;
    Edge_id getSymEdge() const ;
    std::vector<Distribution> getWeight() const;
    Distribution getWeight(const uint32_t& t) const;
    double getPdfValue(const uint32_t& t, const uint32_t& b) const;
    double getCdfValue(const uint32_t& t, const uint32_t& b) const;
    uint32_t getComplexity() const;
    uint32_t getNbOriginalEdge() const ;
    std::vector<Node_id> getMiddleNode() const ;

	/*
	 * Setters
	 */
    void setOrigin(const Node_id& source);
    void setDestination(const Node_id& target);
    void setBW(const bool& forward);
    void setFW(const bool& backward);
    void setDirection(const bool& forward);
    void setSymEdge(const Edge_id& e);
    void setWeight(const uint32_t& t, Distribution&& dist);
    void setComplexity(const uint32_t& c);
    void setMiddleNode(const std::vector<Node_id>& midnod);

    /*
     * aggregate() method: aggregate the current edge with a candidate edge with respect to both cdf
     * if the candidate cdf is more interesting for a given index, set the cdf with the best value and set the corresponding middle node
     */
    bool aggregate(const DynEdge& edge);

    /*
     * makeDummy() method: reset the edge
     */
    void makeDummy();

    /*
     * operator<< : print the current edge on the console (or another output stream)
     * format: (N<src>->N<dest>)[fw/bw] <distrib>
     */
    friend std::ostream& operator<<(std::ostream& os, const DynEdge& edge);

private:
	/*
	 * Attributes
	 */
    bool _forward : 1;
    bool _backward : 1;
    Node_id _origin;
	Node_id _destination;
    Edge_id _symEdge; // Gives the id of the corresponding backward (resp. forward) edge if the current edge is a forward (resp. backward) edge
	std::vector<Distribution> _weight; // Arc weight model, ie a discretized statistical distribution here
	uint32_t _complexity; // Interval of definition of distribution (ie max - min)
	uint32_t _nbOriginalEdge; // Number of original edge that the object represents (=1 if original edge, >1 if shortcut)
	std::vector<Node_id> _midnod; // In case of shortcut, store the middle node id (INVALID_NODE_ID if original edge)
};



#endif /* DATA_GRAPH_DYNEDGE_H_ */
