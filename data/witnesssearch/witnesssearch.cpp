/*
 * witnesssearch.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "../graph/edge.h"
#include "witnesssearch.h"

/*
 * Constructors
 */
WitnessSearch::WitnessSearch(): _graph(), _pq(){}
WitnessSearch::WitnessSearch(Graph* graph): _graph(graph), _pq( graph->getSpecif() ){}

/*
 * Getters
 */

/*
 * run() method: proceed to witness search
 * phase 1: backward interval search to reduce the search space in the backward graph
 * phase 2: sample search to avoid useless profile search
 * phase 3: profile search on the resulting graph to determine if a shortcut is necessary
 * OUTPUT: integer index describing the shortcut needs (undecided: 0, necessary: 1, not necessary: 2)
 */
uint8_t WitnessSearch::run(const Node_id& u_it, const Node_id& x_it, const Node_id& v_it, Distribution& dist_uxv){
//	TRACE("Witness search between nodes " << u_it << " and " << v_it << " (deleted path " << u_it << "->" << x_it << "->" << v_it << " distribution: " << dist_uxv << ")");
	_pq.clearAll();
	/*
	 * --------- Backward interval search ---------
	 */
	backwardIntervalSearch(v_it, x_it, u_it);
//	if(u_it == 10 && v_it == 22){
//		TRACE("TRACE POST BW");
//	}
	// If the origin node has not been reached during backward interval search procedure, it means that there is no witness path between origin and destination nodes => a shortcut is needed
	if ( ! _pq.reached(u_it) ){
		return NECESSARY;
	}
	// Recover the priority queue information about the origin node (first of all, the interval bounds)
	SearchNode& u = _pq.getSearchNode(u_it);
	Interval interval_uv = u.getInterval();
	// If the interval is infinite, that means start node interval has not been updated
	if ( interval_uv.isInfinite() ){
		return NECESSARY;
	}
	// If the final interval upper bound computed between origin and destination is smaller than minimum travel time on path u->x->v, the shortcut is not necessary
	if ( lt(interval_uv.getUB(), dist_uxv.min() ) ){
		return NOT_NECESSARY;
	}
	// If the final interval lower bound computed between origin and destination is larger than maximum travel time on path u->x->v, the shortcut is necessary
	if ( lt(dist_uxv.max(), interval_uv.getLB() ) ){
		return NECESSARY;
	}
	// Clear the priority queue before the next searching procedure
	_pq.clearPQ();
	/*
	 * --------- expectedTimeSearch ---------
	 * Forward expected time search in thinned predecessor graph of backward interval search
	 * Aiming at avoiding superfluous profile search by detecting trivial shortcuts
	 */
	expectedTimeSearch( u_it, v_it );
//	if(u_it == 10 && v_it == 22){
//		TRACE("TRACE POST EXPT");
//	}
	// If the destination node has not been reached during this simple procedure, it means that there is no witness path between origin and destination nodes => a shortcut is needed
	if ( ! _pq.reached(v_it) ){
		return NECESSARY;
	}
	// If the least expected time on path u->x->v is smaller than the LET computed by expectedTimeSearch procedure, we can admit that a shortcut is needed
	double let_uxv = dist_uxv.esp();
	SearchNode& v = _pq.getSearchNode(v_it);
	if ( lt( let_uxv, v.getExpectedTime() ) ){
		return NECESSARY;
	}
	// Clear the priority queue before the next searching procedure
	_pq.clearPQ();
	/*
	 * --------- Profile Search ---------
	 * Forward profile search in thinned predecessor graph of backward interval search
	 */
	profileSearch(u_it, v_it);
//	if(u_it == 10 && v_it == 22){
//		TRACE("TRACE POST PROF");
//	}
	Distribution& dist_uv = v.getDistribution();
	// If the distribution of travel times between origin and destination is invalid, we consider no witness path exists
	if ( dist_uv.isInfinite() ){
		return NECESSARY;
	}
	// If the distribution of u->x->v path is deterministically dominated by the computed distribution, the node contraction can be done without shortcut
	if ( lt( dist_uv.max() , dist_uxv.min() ) ){
		return NOT_NECESSARY;
	}
	// If the distribution of u->x->v path is stochastically dominated by the computed distribution, the node contraction can be done without shortcut
	if( dist_uv.isLargerThan(dist_uxv) ){
//		if(u_it == 10 && v_it == 22){
//			TRACE("IS LARGER THAN");
//		}
		return NOT_NECESSARY;
	}
	// Candidate witness distribution does not dominate the deleted path distribution, a shortcut is needed (at least for some support points)
	return NECESSARY;
}

/*
 * backwardIntervalSearch(const Node_id, const Node_id, const uint32_t&) method: compute final interval label linked to reverse path destination<-start
 * the interval bounds are set as the minimum and maximum of a distribution, namely Q(0) and Q(1), where Q denotes the quantile function
 */
void WitnessSearch::backwardIntervalSearch(const Node_id destination, const Node_id deletedNode, const Node_id start, const uint32_t& nbHops){
	// Initialize the priority queue with destination node and cost 0, set-up the corresponding search node
	SearchNode& d = _pq.insert(destination, 0.0);
	d.setIntervalHop( 0 );
	d.setInterval( 0.0 , 0.0 );
	while ( ! _pq.empty() ){ // While priority queue is not empty, continue the process
		// The algorithm may stop if provided start node is reached (it is the case only if the associated interval upper bound is lower than the lowest value in the PQ)
		if ( _pq.reached(start) ){
			SearchNode& s = _pq.getSearchNode(start);
			if ( !s.getInterval().isInfinite() && ge( _pq.getMinPriority(), s.getInterval().getUB() ) ){
				return;
			}
		}
		// Set the current search node with the PQ minimum value
		SearchNode& u = _pq.deleteMin();
		// If this node is too far from the destination, it is ignored (because of hop limit)
		if ( u.getIntervalHop() >= nbHops ){
			continue;
		}
		const Node_id u_it = u.getNodeId();
		const double u_lower = u.getInterval().getLB();
		const double u_upper = u.getInterval().getUB();
		const uint32_t u_hops = u.getIntervalHop();
		const Node_id u_id = _pq.getNodeId(u);
		// For each edge of the backward graph (ie for each incoming edge to this node), relax
		for ( Edge_id e = _graph->getNodeBeginBW(u_it) ; e != _graph->getNodeEndBW(u_it) ; ++e ){
			// Recover the edge structure
			Edge curEdge( _graph->getBwEdges()[e] );
			// Recover the origin node and the distribution corresponding to the current edge
			const Node_id v_it = curEdge.getOrigin();
			if(v_it == deletedNode ){
				continue;
			}
			if( v_it == u.getPredId() ){
				continue;
			}
			Distribution dist_vu(curEdge.getWeight());
			const double min_e = dist_vu.min();
			const double max_e = dist_vu.max();
			// Compute the candidate interval
			const Interval interval_v_new(u_lower + min_e, u_upper + max_e);
			// If origin node is not yet is the priority queue, put it in with the candidate interval lower bound as its priority coefficient (and configure the corresponding search node structure)
			if ( ! _pq.reached(v_it) ){
				SearchNode& v = _pq.insert(v_it, interval_v_new.getLB(), u_it);
				v.setIntervalHop( u_hops + 1 );
				v.setInterval( interval_v_new.getLB() , interval_v_new.getUB() );
				// Set the predecessor information to remember the origin of the node configuration
				v.setBoundingPredecessors(e, u_id, e, u_id);
			}
			// If the origin node has still been added in the PQ...
			else{
				// ...set the predecessor information only if the interval bounds are decreased
				SearchNode& v = _pq.getSearchNode(v_it);
				// Case with no amelioration
				if ( ge( interval_v_new.getLB() , v.getInterval().getLB() ) && ge( interval_v_new.getUB() , v.getInterval().getUB() ) ){
					// Case with equality (no improvement, but diversification of predecessors -> let consider a new best predecessor for upper bound)
					if ( eq( interval_v_new.getLB() , v.getInterval().getLB() ) && eq( interval_v_new.getUB() , v.getInterval().getUB() ) ){
						v.setPredecessorUB(e, u_id);
					}
					continue;
				}
				// Upper bound improvement case
				if ( lt( interval_v_new.getUB() , v.getInterval().getUB() ) ){
					v.setPredecessorUB(e, u_id);
				}
				// Lower bound improvement case
				if ( lt( interval_v_new.getLB() , v.getInterval().getLB() ) ){
					v.setPredecessorLB(e, u_id);
				}
				// Hop number is modified only if the new candidate path is longer
				v.setIntervalHop( std::max(uint8_t(u_hops + 1), v.getIntervalHop() ) );
				// Update the interval if it is improved by the candidate (call to merge procedure)
				const Interval interval_merged = merge( v.getInterval() , interval_v_new );
				v.setInterval( interval_merged.getLB(), interval_merged.getUB() );
				// If origin node is not yet in the PQ, re-insert it
				if ( ! _pq.contains(v) ){
					_pq.insertAgain( v , interval_merged.getLB() );
					v.setPredId( u_it );
				}
				// Else (the node is still in the PQ), decreased the corresponding priority coefficient
				else{
					_pq.decrease( v , interval_merged.getLB() );
					v.setPredId( u_it );
				}
			}
		}
	} // End of procedure
}

/*
 * expectedTimeSearch(const Node_id, const Node_id, const uint32_t&) method: compute the least expected time for path start->destination
 * starting from the node label computed by backward interval search, this procedure investigates on the least expected time path between start and destination in the predecessor graph
 */
void WitnessSearch::expectedTimeSearch(const Node_id& start, const Node_id& destination, const uint32_t& nbHops){
	// Initialize the priority queue with start node and a cost corresponding to the node lower bound, set-up the corresponding search node
	SearchNode& s = _pq.getSearchNode(start);
	s.setSampleHop( 0 );
	s.setExpectedTime( 0 );
	_pq.insertAgain(s, s.getInterval().getLB() );
	while ( ! _pq.empty() ){ // While priority queue is not empty, continue the process
		SearchNode& d = _pq.getSearchNode(destination);
		// The algorithm stops if the priority queue minimum is larger than destination coefficient (and by the way if this coefficient is not infinite)
		if ( d.getExpectedTime() != std::numeric_limits<double>::max() && ge( _pq.getMinPriority(), d.getExpectedTime() ) ){
			return;
		}
		// Delete the minimum element of the priority queue, and set the current node as the corresponding node
		SearchNode& u = _pq.deleteMin();
		// If the accepted distance between origin and destination is overtaken, the candidate node can't be part of a witness path
		if ( u.getSampleHop() >= nbHops ){
			continue;
		}
		// If predecessors of current node u are the same (ie predecessors were computed during backward interval search just before), set one value as invalid (not necessary to compute twice the same thing)
		if ( u.getPredecessorLB()._bw_edge_it == u.getPredecessorUB()._bw_edge_it ){
			u.setPredecessorUB(INVALID_EDGE_ID,u.getPredecessorUB()._search_node_id);
		}
		// For each predecessors of search node u (predecessor which gave the interval lower and upper bounds, respectively)
		for ( auto& pred : { u.getPredecessorLB(), u.getPredecessorUB() } ){
			const Edge_id e = pred._bw_edge_it;
			// If the predecessor correspond to an invalid value, skip the current loop
			if ( e == INVALID_EDGE_ID ){
				continue;
			}
			Edge curEdge( _graph->getBwEdges()[e] );
			// Set the node v as the next candidate node (edge u->v will be relaxed during the search)
			SearchNode& v = _pq.getSearchNodeFromId(pred._search_node_id);
			Distribution dist_uv = curEdge.getWeight();
			// Compute the candidate label let_v_new
			const double let_u = u.getExpectedTime();
			const double let_v_new = dist_uv.esp() + let_u;
			// If let for node v has not been visited yet, just update it as usual
			if ( v.getExpectedTime() == std::numeric_limits<double>::max() ){
				_pq.insertAgain(v, let_v_new + v.getInterval().getLB());
				v.setSampleHop( u.getSampleHop() + 1 );
				v.setExpectedTime( let_v_new );
			}
			// Else, a comparison with existing best label must be accomplished
			else{
				// First case: no improvement, the loop is skipped
				if ( ge( let_v_new, v.getExpectedTime() ) ){
					continue;
				}
				// Else, search node has to be updated
				v.setSampleHop( u.getSampleHop() + 1 );
				v.setExpectedTime( let_v_new );
				// If node v is in the priority queue, decrease the corresponding priority coefficient
				if ( _pq.contains(v) ){
					_pq.decrease(v, let_v_new + v.getInterval().getLB());
				}
				// Else re-insert it
				else{
					_pq.insertAgain(v, let_v_new + v.getInterval().getLB());
				}
			}
		}
	}
	return; // End of procedure
}

/*
 * profileSearch(const Node_id&, const Node_id&, const uint32_t&) method:
 */
void WitnessSearch::profileSearch(const Node_id& start, const Node_id& destination, const uint32_t& nbHops){
	// Initialize the priority queue with start node and cost 0, set-up the corresponding search node
	SearchNode& s = _pq.getSearchNode(start);
	s.setDistribution( Distribution(_pq.getNbPts()+1,_pq.getDelta(),false) ); // Initialize the distribution: consider 0 as a certain value to reach the start node
	s.setProfileHop( 0 );
	_pq.insertAgain( s, s.getInterval().getLB() );
	SearchNode& d = _pq.getSearchNode(destination);
	while ( ! _pq.empty() ){ // While priority queue is not empty, continue the process
		// The algorithm stops if the priority queue minimum is larger than destination maximum possible value (with a valid distribution at destination node)
		if ( ! d.infiniteDistrib() && gt( _pq.getMinPriority(), d.getDistMax() ) ){
			return;
		}
		// Set the current search node with the PQ minimum value
		SearchNode& u = _pq.deleteMin();
//		if(start == 10 && destination == 22){
//			CONTINUE_STATUS("[BW] Node N" << u.getNodeId() << "\t");
//		}
		// If this node is too far from the destination, it is ignored (because of hop limit)
		if ( u.getProfileHop() >= nbHops ){
			continue;
		}
		// If predecessors of current node u are the same (ie predecessors were computed during backward interval search just before), set one value as invalid (it is useless to compute twice the same thing)
		if ( u.getPredecessorLB()._bw_edge_it == u.getPredecessorUB()._bw_edge_it ){
			u.setPredecessorUB(INVALID_EDGE_ID,u.getPredecessorUB()._search_node_id);
		}
		// For each of both predecessors of search node u (predecessors which gave the interval lower and upper bounds, respectively)
		for ( auto pred : { u.getPredecessorLB(), u.getPredecessorUB() } ){
			const Edge_id e = pred._bw_edge_it;
			// If the predecessor correspond to an invalid value, skip the current loop
			if ( e == INVALID_EDGE_ID ){
				continue;
			}
			Edge curEdge( _graph->getBwEdges()[e] );
			// Set the node v as the next candidate node (edge u->v will be relaxed during the search)
			SearchNode& v = _pq.getSearchNodeFromId( pred._search_node_id );
//			if(start == 10 && destination == 22){
//				CONTINUE_STATUS("->N" << v.getNodeId() << "\t");
//			}
			Distribution dist_uv = curEdge.getWeight();
			// If the distribution is valid (values are distributed on the interval [0,Tmax], and not only equal to Tmax) -- ie if node v distribution as still be updated
			if ( ! v.infiniteDistrib() ){
				// If minimum possible value after edge u->v relaxation is larger than maximum value of node v distribution, the current candidate edge is skipped
				if ( ge(u.getDistMin() + dist_uv.min() , v.getDistMax()) ){
					continue;
				}
				// Same conclusion with interval comparison: if candidate priority queue coefficient does not improve worst existing priority coefficient at node v, skip the candidate
				if ( gt(u.getDistMin() + dist_uv.min() + v.getInterval().getLB() , v.getDistMax() + v.getInterval().getUB() )	){
					continue;
				}
			}
			// If distributions are overlapping, convolution process can not be avoided
			Distribution dist_v_new = u.getDistribution().convolute( dist_uv );
			// If v has never been visited, the convoluted distribution becomes its own distribution
			if ( v.infiniteDistrib() ){
				_pq.insertAgain(v, dist_v_new.min() + v.getInterval().getLB());
				v.setDistribution( std::move(dist_v_new) );
				v.setProfileHop( u.getProfileHop() + 1 );
			}
			// Else, v has already been visited, the characteristics of this search node must be updated (if necessary)
			else{
				// First case: minimum possible value of candidate distribution is larger than existing distribution worst value, no improvement, candidate must be skipped
				// **************** Deterministic dominance, can be replaced by stochastic dominance to detect more no-improvement cases (warning: stochastic dominance is costly...)
				if ( ge(dist_v_new.min(), v.getDistMax()) ){
					continue;
				}
				// Both distributions (candidate and current best) can be mixed to find the final best
				Distribution dist_v_merge = dist_v_new.aggregate( v.getDistribution() );
				v.setDistribution( std::move(dist_v_merge) );
				// Hop number is modified only if the new candidate path is longer
				v.setProfileHop( std::max(v.getProfileHop(), uint8_t(u.getProfileHop() + 1) ) );
				// If v node is not yet in the PQ, re-insert it
				if ( ! _pq.contains(v) ){
					_pq.insertAgain(v, dist_v_merge.min() + v.getInterval().getLB());
				}
				// Else (the node is still in the PQ), decreased the corresponding priority coefficient
				else{
					_pq.decrease(v, dist_v_merge.min() + v.getInterval().getLB());
				}
			}
		}
//		if(start == 10 && destination == 22){
//			CONTINUE_STATUS("\n");
//		}
	} // End of procedure
}
