/*
 * edge.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "edge.h"

/*
 * Constructors
 */
Edge::Edge(): _forward(false),_backward(false), _origin(INVALID_NODE_ID),_destination(INVALID_NODE_ID), _symEdge(INVALID_EDGE_ID), _complexity(0), _nbOriginalEdge(1),_midnod(INVALID_NODE_ID){}
Edge::Edge(const bool& fw):
						_forward(fw), _backward(!fw), _origin(INVALID_NODE_ID), _destination(INVALID_NODE_ID), _symEdge(INVALID_EDGE_ID), _complexity(0), _nbOriginalEdge(1),_midnod(INVALID_NODE_ID){}
Edge::Edge(const bool& fw, const Node_id& src, const Node_id& dest):
								_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(INVALID_EDGE_ID), _complexity(0), _nbOriginalEdge(1), _midnod(INVALID_NODE_ID){}
Edge::Edge(const bool& fw, const Node_id& src, const Node_id& dest, Distribution& dist):
								_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(INVALID_EDGE_ID), _weight(dist),
								_complexity( dist.range() ), _nbOriginalEdge(1), _midnod(INVALID_NODE_ID){}
Edge::Edge(const bool& fw, const Node_id& src, const Node_id& dest, Distribution& dist, const Node_id& midnod): _forward(fw),_backward(!fw), _origin(src),_destination(dest),
		_symEdge(INVALID_EDGE_ID), _weight(dist), _complexity(dist.range()), _nbOriginalEdge(1), _midnod(midnod){}
Edge::Edge(const bool& fw, const Node_id& src, const Node_id& dest, Distribution& dist, const uint32_t& complex, const uint32_t& origedge, const Node_id& midnod):
						_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(INVALID_EDGE_ID), _weight(dist), _complexity(complex), _nbOriginalEdge(origedge), _midnod(midnod){}
Edge::Edge(const bool& fw, const Node_id& src, const Node_id& dest, const Edge_id& e, Distribution& dist, const Node_id& midnod):
						_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(e), _weight(dist), _complexity( dist.range() ), _nbOriginalEdge(1), _midnod(midnod){}
Edge::Edge(const bool& fw, const Node_id& src, const Node_id& dest, const Edge_id& e, Distribution& dist):
						_forward(fw), _backward(!fw), _origin(src), _destination(dest), _symEdge(e), _weight(dist), _complexity( dist.range() ), _nbOriginalEdge(1), _midnod(INVALID_NODE_ID){}
Edge::Edge(const Edge& edge){
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
bool Edge::getFW() const { return _forward; }
bool Edge::getBW() const { return _backward; }
Node_id Edge::getOrigin() const { return _origin; }
Node_id Edge::getDestination() const { return _destination; }
Edge_id Edge::getSymEdge() const { return _symEdge; }
std::array<double,3> Edge::getWeightT(const uint32_t& t) const {
	std::array<double,3> weight;
	weight[0] = _weight.getSupT(t) ;
	weight[1] = _weight.getPdfT(t) ;
	weight[2] = _weight.getCdfT(t) ;
	return weight;
}
std::array<double,3> Edge::getLastWeight() const{
	std::array<double,3> weight;
	weight[0] = _weight.getSup().back() ;
	weight[1] = _weight.getPdf().back() ;
	weight[2] = _weight.getCdf().back() ;
	return weight;
}
Distribution Edge::getWeight() const { return _weight; }
double Edge::getMinWeight() const { return _weight.min() ; }
double Edge::getMaxWeight() const { return _weight.max() ; }
uint32_t Edge::getComplexity() const { return _complexity; }
uint32_t Edge::getNbOriginalEdge() const { return _nbOriginalEdge; }
Node_id Edge::getMiddleNode() const { return _midnod; }

/*
 * Setters
 */
void Edge::setFW(const bool& forward) { _forward = forward; }
void Edge::setBW(const bool& backward) { _backward = backward; }
void Edge::setDirection(const bool& forward){ _forward = forward; _backward = !forward; }
void Edge::setOrigin(const Node_id& source) { _origin = source; }
void Edge::setDestination(const Node_id& target) { _destination = target; }
void Edge::setSymEdge(const Edge_id& e){ _symEdge = e; }
void Edge::setWeight(Distribution&& dist) { _weight = std::move(dist); }
void Edge::setWeight(const double c) { _weight = Distribution(c); }
void Edge::setComplexity(const uint32_t& c){ _complexity = c; }
void Edge::setMiddleNode(const Node_id& midnod){ _midnod = midnod; }

/*
 * aggregate() method: aggregate the current edge with a candidate edge with respect to both cdf
 * if the candidate cdf is more interesting for a given index, set the cdf with the best value and set the corresponding middle node
 * return true if candidate distribution dominates current one
 */
bool Edge::aggregate(const Edge& candidateEdge){
	bool distImprovement(true);
	for(uint32_t t(0) ; t < getWeight().getSize() ; ++t){
		double candidateCdfValue( candidateEdge.getWeightT(t)[2] );
		if( lt( _weight.getCdfT(t) , candidateCdfValue ) ){
			_weight.setCdfT( t , candidateCdfValue );
		}
		if( lt( candidateCdfValue , _weight.getCdfT(t) ) ){
			distImprovement = false;
		}
		if(t == 0){
			_weight.setPdfT( t , _weight.getCdfT(t) );
		}else{
			_weight.setPdfT( t , _weight.getCdfT(t)-_weight.getCdfT(t-1) );
		}
	}
	return distImprovement;
}

/*
 * makeDummy() method: reset the edge
 */
void Edge::makeDummy(){
	_forward = false;
	_backward = false;
	_origin = INVALID_NODE_ID;
	_destination = INVALID_NODE_ID;
	_symEdge = INVALID_EDGE_ID;
	_complexity = 0;
	_nbOriginalEdge = 1;
	_weight.reset();
	_midnod = INVALID_NODE_ID;
}

/*
 * << operator: return an outstream version of the edge (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const Edge& edge) {
	std::string direction("dum");
	if(edge._forward && !edge._backward){
		direction = "FW";
	}
	if(!edge._forward && edge._backward){
		direction = "BW";
	}
	os << "(N" << edge._origin << "-(n" << edge._midnod << ")->N" << edge._destination << ")[" << direction << "] " << edge._weight;
	return os;
}
