/*
 * dynedge.cpp
 *
 *  Created on: 28 avr. 2016
 *      Author: delhome
 */

#include "dynedge.h"

/*
 * Constructors
 */
DynEdge::DynEdge(): _forward(false),_backward(false), _origin(INVALID_NODE_ID),_destination(INVALID_NODE_ID), _symEdge(INVALID_EDGE_ID), _complexity(0), _nbOriginalEdge(1),_midnod(INVALID_NODE_ID){}
DynEdge::DynEdge(const bool& fw):
						_forward(fw), _backward(!fw), _origin(INVALID_NODE_ID), _destination(INVALID_NODE_ID), _symEdge(INVALID_EDGE_ID), _complexity(0), _nbOriginalEdge(1),_midnod(INVALID_NODE_ID){}
DynEdge::DynEdge(const bool& fw, const Node_id& src, const Node_id& dest):
								_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(INVALID_EDGE_ID), _complexity(0), _nbOriginalEdge(1), _midnod(INVALID_NODE_ID){}
DynEdge::DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, Distribution& dist):
								_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(INVALID_EDGE_ID), _weight(1,dist),
								_complexity( dist.range() ), _nbOriginalEdge(1), _midnod(dist.getSize(),INVALID_NODE_ID){}
DynEdge::DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, std::vector<Distribution>& dist, const Node_id& midnod): _forward(fw),_backward(!fw), _origin(src),_destination(dest),
		_symEdge(INVALID_EDGE_ID), _nbOriginalEdge(1), _midnod(dist.begin()->getSize(), midnod ){
	uint32_t lb(0);
	uint32_t ub(dist.begin()->getSup().back());
	for(auto& d: dist){
		_weight.push_back(d);
		if(d.min() > lb){
			lb = d.min();
		}
		if(d.max() < ub){
			ub = d.max();
		}
	}
	_complexity = ub - lb;
}
DynEdge::DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, Distribution& dist, const uint32_t& complex, const uint32_t& origedge, const std::vector<Node_id>& midnod):
						_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(INVALID_EDGE_ID), _weight(1,dist), _complexity(complex), _nbOriginalEdge(origedge), _midnod(midnod){}
DynEdge::DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, const Edge_id& e, std::vector<Distribution>& dist, const Node_id& midnod):
						_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(e), _nbOriginalEdge(1), _midnod(dist.begin()->getSize(), midnod ){
	uint32_t lb(0);
	uint32_t ub(dist.begin()->getSup().back());
	for(auto& d: dist){
		_weight.push_back(d);
		if(d.min() > lb){
			lb = d.min();
		}
		if(d.max() < ub){
			ub = d.max();
		}
	}
	_complexity = ub - lb;
}
DynEdge::DynEdge(const bool& fw, const Node_id& src, const Node_id& dest, const Edge_id& e, Distribution& dist):
						_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(e), _weight(1,dist), _complexity( dist.range() ), _nbOriginalEdge(1), _midnod(dist.getSize(), INVALID_NODE_ID ){}
DynEdge::DynEdge(const DynEdge& edge){
	_forward = edge.getFW();
	_backward = edge.getBW();
	_origin = edge.getOrigin();
	_destination = edge.getDestination();
	_symEdge = edge.getSymEdge();
	_weight = edge.getWeight();
	_complexity = edge.getComplexity();
	_nbOriginalEdge = edge.getNbOriginalEdge();
	_midnod = edge.getMiddleNode();
}

/*
 * Getters
 */
bool DynEdge::getFW() const { return _forward; }
bool DynEdge::getBW() const { return _backward; }
Node_id DynEdge::getOrigin() const { return _origin; }
Node_id DynEdge::getDestination() const { return _destination; }
Edge_id DynEdge::getSymEdge() const { return _symEdge; }
std::vector<Distribution> DynEdge::getWeight() const { return _weight; }
Distribution DynEdge::getWeight(const uint32_t& t) const { return _weight[t]; }
double DynEdge::getPdfValue(const uint32_t& t, const uint32_t& b) const { return _weight[t].getCdfT(b); }
double DynEdge::getCdfValue(const uint32_t& t, const uint32_t& b) const { return _weight[t].getPdfT(b); }
uint32_t DynEdge::getComplexity() const { return _complexity; }
uint32_t DynEdge::getNbOriginalEdge() const { return _nbOriginalEdge; }
std::vector<Node_id> DynEdge::getMiddleNode() const { return _midnod; }

/*
 * Setters
 */
void DynEdge::setFW(const bool& forward) { _forward = forward; }
void DynEdge::setBW(const bool& backward) { _backward = backward; }
void DynEdge::setDirection(const bool& forward){ _forward = forward; _backward = !forward; }
void DynEdge::setOrigin(const Node_id& source) { _origin = source; }
void DynEdge::setDestination(const Node_id& target) { _destination = target; }
void DynEdge::setSymEdge(const Edge_id& e){ _symEdge = e; }
void DynEdge::setComplexity(const uint32_t& c){ _complexity = c; }
void DynEdge::setMiddleNode(const std::vector<Node_id>& midnod){ _midnod = midnod; }

/*
 * << operator: return an outstream version of the edge (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const DynEdge& edge) {
	std::string direction("dum");
	if(edge._forward && !edge._backward){
		direction = "FW";
	}
	if(!edge._forward && edge._backward){
		direction = "BW";
	}
	os << "(N" << edge._origin << "-(n" << edge._midnod.front() << ")->N" << edge._destination << ")[" << direction << "] " << edge._weight.front();
	return os;
}
