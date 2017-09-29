/*
 * schquery.cpp
 *
 *  Created on: 1 mars 2016
 *      Author: delhome
 */

#include "schquery.h"

/*
 * Constructors
 */
SCHQuery::SCHQuery(Graph* graph): _graph(graph), _start(INVALID_NODE_ID), _destination(INVALID_NODE_ID), _forwardDirection( false ), _targettedProb(std::numeric_limits<double>::max()),
_timeBudget(std::numeric_limits<double>::max()), _uppB(std::numeric_limits<double>::max()){
	_priorityQueues[0] = QueryContext(_graph->getSpecif()); // Initialize backward priority queue
	_priorityQueues[1] = QueryContext(_graph->getSpecif()); // Initialize forward priority queue
}

/*
 * Getters
 */
bool SCHQuery::getDirection(){ return _forwardDirection; }

/*
 * flip() method: set _forwardDirection as true if it is false, as false otherwise and return the new value
 */
bool SCHQuery::flip(){
	_forwardDirection = !_forwardDirection;
	return _forwardDirection;
}

/*
 * oneToOne() method: compute the shortest path between given origin and destination nodes
 * Work into the ordered graph
 */
RoutingPolicy SCHQuery::oneToOne(const Node_id& start, const Node_id& destination){
	//	STATUS("Shortest path query between N" << start << " and N" << destination << "...\n");
	_timeBudget = std::numeric_limits<double>::max();
	_start = start;
	_destination = destination;
	if ( start == destination ){
		return RoutingPolicy(_destination,_graph->getSpecif().getNbPts(),_graph->getSpecif().getDelta());
	}
	_candidates.clear();
	_priorityQueues[_forwardDirection].clearAll();
	_priorityQueues[!_forwardDirection].clearAll();
	_forwardDirection = false;
	_uppB = std::numeric_limits<double>::max();
	bidirectionalProfileSearch();
//	STATUS("End of bidirectional search" << "\n");
	// Here the routing policies are identified in each search direction they have to be merged into a single routing policy
	RoutingPolicy destinationPolicy = buildSolution(_candidates);
//	STATUS("End of solution building" << "\n");
	// Develop paths to retrieve all physical nodes that are visited by optimal routing policy
	destinationPolicy.developPaths( _graph );
//	STATUS("All paths are developed - solution ready" << "\n");
//	MARK("Routing policy toward node N" << _destination << " is the following:\n" << destinationPolicy);
	return destinationPolicy;
}

/*
 * bidirectionalProfileSearch() method: compute the stochastic shortest path by the way of a bidirectional profile search
 * The algorithm can work on the whole hierarchized graph, or more efficiently, in the reduced corridor given by bidirectionalIntervalSearch() method
 * Give the best travel time distribution to link start and destination nodes and associated predecessor graph (paths may be rebuilt subsequently)
 */
void SCHQuery::bidirectionalProfileSearch(){
	// Initialization: set two search nodes corresponding to origin and destination
	_priorityQueues[!_forwardDirection].init(_start, 0.0 );
	_priorityQueues[_forwardDirection].init(_destination, 0.0 );
	// Main loop: continue the search while forward and backward priority queue contain any element
//	int counter(0);
	while( (! _priorityQueues[_forwardDirection].empty() || ! _priorityQueues[!_forwardDirection].empty()) ){
		if( ! _priorityQueues[_forwardDirection].empty() && ! _priorityQueues[!_forwardDirection].empty() ){
			if( gt(std::min(_priorityQueues[_forwardDirection].getMinPriority(),_priorityQueues[!_forwardDirection].getMinPriority()),_uppB) ){
//								TRACE("BREAK -- Upper bound comparison...");
				break;
			}
		}
		// If opposite priority queue is not empty, flip to the opposite search direction (guarantee the search direction alternance)
		if( !_priorityQueues[!_forwardDirection].empty() ){
			//			TRACE("Change search direction!");
			flip();
		}
		QuerySearchNode& u = _priorityQueues[_forwardDirection].deleteMin();
		const Node_id u_id = u.getNodeId();
//		TRACE( "[" << (_forwardDirection?"FORWARD":"BACKWARD") << " search] Current node: Node N" << u_id);
		QuerySearchNode ubis = QuerySearchNode( u_id , _graph->getSpecif().getNbPts()+1, _graph->getSpecif().getDelta() );
		if( _priorityQueues[!_forwardDirection].reached( u_id ) ){
			ubis = _priorityQueues[!_forwardDirection].getSearchNode( u_id ); // Recover current node information in opposite search
		}
//		TRACE("Node N"<< u_id << " in the other PQ: " << ubis);
		// If distributions at node u are already initialized, there may be a candidate node set update
		if( !u.getPolicy().empty() && !ubis.getPolicy().empty() ){
//			TRACE("CANDIDATE NEW UPPER BOUND: " << ubis.getDistMax() << "+" << u.getDistMax() << "=" << (double) ubis.getDistMax() + u.getDistMax() );
			// Update upper bound with node upper bounds (respectively in opposite and current direction search)
			_uppB = std::min( _uppB, (double) ubis.getDistMax() + u.getDistMax() );
//			TRACE("Node N" << u_id << " has a distribution defined in [" << ubis.getDistMin() << ";" << ubis.getDistMax() << "] in the other PQ.");
//			TRACE("Valid interval for current node");
			if( lt( _uppB , std::numeric_limits<double>::max() ) && le( u.getDistMin()+ubis.getDistMin() , _uppB ) ){
//				TRACE( "NODE LOWER BOUNDS: " << u.getDistMin() << "+" << ubis.getDistMin() << "=" << u.getDistMin()+ubis.getDistMin() << " vs UPPER BOUND: " << _uppB << " --> NEW CANDIDATE: node N" << u_id);
				_candidates.insert( u_id );
			}
		}
		if( u_id == _destination && _forwardDirection ){
//			TRACE("DESTINATION NODE REACHED DURING FORWARD SEARCH!");
			continue;
		}
		if( u_id == _start && !_forwardDirection ){
//			TRACE("ORIGIN NODE REACHED DURING BACKWARD SEARCH!");
			continue;
		}
		//		TRACE("Upper bound of the search: " << _uppB);
		// Relax each incident edge: outcoming forward edges for forward search, incoming backward edges for backward search
		if( _forwardDirection ){
			for ( Edge_id e = _graph->getNodeBeginFW(u_id) ; e != _graph->getNodeEndFW(u_id) ; ++e ){
				Edge curEdge( _graph->getFwEdge(e) );
				const Node_id v_id = curEdge.getDestination();
				// If node v has a lower level, ignore it (upward search)
//				TRACE("Node N" << u_id << " has level L" << _graph->getLevel(u_id) << " --> Node N" << v_id << " has level L" << _graph->getLevel(v_id) << (_graph->getLevel(v_id) < _graph->getLevel(u_id)?" --> CANCEL EVALUATION":"") );
				if( _graph->getLevel(v_id) < _graph->getLevel(u_id) ){
					continue;
				}
//				TRACE("Current edge: " << curEdge);
				// Set the current loop information: edge u->v distribution and node v
				Distribution dist_uv( curEdge.getWeight() );
				double candidateValue( u.getDistMin()+dist_uv.min() );
				RoutingPolicy predPolicy = u.getPolicy();
				// If there is not any policy for node v
				if( !_priorityQueues[_forwardDirection].reached(v_id) ){
					// Insert candidate node into the priority queue with the shortest possible travel time of the policy
					_priorityQueues[_forwardDirection].insert( v_id , candidateValue , predPolicy , e , dist_uv );
				}
				// Otherwise there is an existing policy for candidate node v
				else{
					bool updatePQ( _priorityQueues[_forwardDirection].managePolicy( v_id , candidateValue , predPolicy , e , dist_uv ) );
					if( updatePQ ){
						_priorityQueues[_forwardDirection].update( v_id , candidateValue );
					}
				}
			} /* End for loop */
		} /* End if predicate */
		else{
			for ( Edge_id e = _graph->getNodeBeginBW(u_id) ; e != _graph->getNodeEndBW(u_id) ; ++e ){
				Edge curEdge( _graph->getBwEdge(e) );
				const Node_id v_id = curEdge.getOrigin();
				// If node v has a lower level, ignore it (upward search)
//				TRACE("Node N" << u_id << " has level L" << _graph->getLevel(u_id) << " --> Node N" << v_id << " has level L" << _graph->getLevel(v_id) << (_graph->getLevel(v_id) < _graph->getLevel(u_id)?" --> CANCEL EVALUATION":"") );
				if( _graph->getLevel(v_id) < _graph->getLevel(u_id) ){
					continue;
				}
//				TRACE("Current edge: " << curEdge);
				// Set the current loop information: edge u->v distribution and node v
				Distribution dist_uv( curEdge.getWeight() );
				double candidateValue( u.getDistMin()+dist_uv.min() );
				RoutingPolicy predPolicy = u.getPolicy();
				// If there is not any policy for node v
				if( !_priorityQueues[_forwardDirection].reached(v_id) ){
					// Insert candidate node into the priority queue with the shortest possible travel time of the policy
					_priorityQueues[_forwardDirection].insert( v_id , candidateValue , predPolicy , e , dist_uv );
				}
				// Otherwise there is an existing policy for candidate node v
				else{
					bool updatePQ( _priorityQueues[_forwardDirection].managePolicy( v_id , candidateValue , predPolicy , e , dist_uv ) );
					if( updatePQ ){
//						TRACE("Update PQ...");
						_priorityQueues[_forwardDirection].update( v_id , candidateValue );
					}
				}
			} /* End for loop */
//			CONTINUE_STATUS( _priorityQueues[_forwardDirection] );
		} /* End else predicate */
//		TRACE("LOOP END -- " << (_forwardDirection?"FORWARD":"BACKWARD"));
//		TRACE("Empty PQ? BW:" << (_priorityQueues[0].empty()?"YES":"NO") << " - FW:" << (_priorityQueues[1].empty()?"YES":"NO") );
//		TRACEF("Minimal priority keys?");
//		if( !_priorityQueues[0].empty() ){
//			CONTINUE_STATUS(" BW:" << _priorityQueues[0].getMinPriority());
//		}
//		if( !_priorityQueues[1].empty() ){
//			CONTINUE_STATUS(" FW:" << _priorityQueues[1].getMinPriority());
//		}
//		CONTINUE_STATUS("\n");
//		TRACEF("Candidate node set: {");
//		for(auto& n: _candidates){
//			CONTINUE_STATUS(" " << n);
//		}
//		CONTINUE_STATUS(" }\n");
	} /* End while loop */
//	STATUS("End of bidirectional search after " << counter << " loops.\n");
}

/*
 * buildSolution(set<Node_id>) method: build the final routing policy to join the destination, by merging the routing policy using each search meeting nodes
 */
RoutingPolicy SCHQuery::buildSolution(std::set<Node_id> meetingNodes){
//	TRACE("Build final routing policy to reach destination...");
//	TRACEF("Candidate node set: {");
//	for(auto& n: _candidates){
//		CONTINUE_STATUS(" " << n);
//	}
//	CONTINUE_STATUS(" }\n");
	std::set<Node_id>::iterator itmn = meetingNodes.begin();
	RoutingPolicy solution = _priorityQueues[1].getSearchNode(*itmn).appendPolicy( _priorityQueues[0].getSearchNode(*itmn) );
//	TRACE("Routing policy with paths meeting at node N" << *itmn << ".\n" << solution );
	for( ++itmn ; itmn != meetingNodes.end() ; ++itmn ){
		RoutingPolicy candidatePolicy = _priorityQueues[1].getSearchNode(*itmn).appendPolicy( _priorityQueues[0].getSearchNode(*itmn) );
//		TRACE("Routing policy with paths meeting at node N" << *itmn << ".\n" << candidatePolicy );
		solution.merge( candidatePolicy );
//		TRACE("Merged routing policy:\n" << candidatePolicy );
	}
//	TRACE("[BEFORE CLEANING] " << solution);
	solution.clean( _graph->getSpecif() );
//	TRACE("[AFTER CLEANING] " << solution);
	return solution;
}

/*
 * operator << : send to an output stream the SCHQuery structure
 */
std::ostream& operator<<(std::ostream& os, SCHQuery& schq){
	return os;
}
