/*
 * ordering.cpp
 *
 *  Created on: 19 févr. 2016
 *      Author: delhome
 */


#include <algorithm>
#include <unordered_map>

#include "ordering.h"

#include "../../data_io/edge_io.h"
#include "../../data_io/hierarchy_io.h"

/*
 * Constructors
 */
Ordering::Ordering(): _graph(), _wcache(0), _firstWorkingNode( 0 ), _lastWorkingNode( 0 ){}
Ordering::Ordering(Graph* g): _graph( g ), _wcache( g->getNbNodes() ), _nodeIds( g->getNbNodes() , INVALID_NODE_ID ), _nextNodeToContract( g->getNbNodes() , false ), _contractionCost( g->getNbNodes() , 0 ),
		_nodeDepth( g->getNbNodes() , 1 ), _firstWorkingNode( 0 ), _lastWorkingNode( 0 ){
	std::iota(_nodeIds.begin(),_nodeIds.end(), 0);
}

/*
 * Getters
 */
LocalThread& Ordering::getLocThread(const uint32_t& index) { return _localThreads[index]; }
double Ordering::getContractionCost(const Node_id& u){ return _contractionCost[u]; }
std::vector<Node_id> Ordering::getNodeIds() const{ return _nodeIds; }

/*
 * Setters
 */
void Ordering::setConfig(double param_eq, double param_d, double param_oeq, double param_cq){
	// Change the node ordering configuration parameters (priority coefficients associated with each criterion)
	_config.setParamEq(param_eq);
	_config.setParamSsd(param_d);
	_config.setParamOeq(param_oeq);
	_config.setParamCq(param_cq);
}

/*
 * simulateContraction(Node_id) method: initialize the hierarchy building by setting contraction cost of each node
 * Node x contraction is simulated in order to evaluate the priority coefficient value at the beginning of the process
 */
void Ordering::simulateContraction(const Node_id& n){
	uint32_t nbInsEdge(0), nbRemEdge(0), nbOriginEdgeInsert(0), nbOriginEdgeRemove(0), complexityInsert(0), complexityRemove(0);
	// (INSERTION INDICATORS) Loop over all edges and evaluation of the necessity to add shortcuts
	for ( Edge_id e_in = _graph->getNodeBeginBW(n) ; e_in < _graph->getNodeEndBW(n) ; ++e_in ){
		for ( Edge_id e_out = _graph->getNodeBeginFW(n) ; e_out < _graph->getNodeEndFW(n) ; ++e_out ){
			// If current u->v->w path is such that u=w, no shortcut (trivial)
			if ( _graph->getBwEdge(e_in).getOrigin() == _graph->getFwEdge(e_out).getDestination() ){
				continue;
			}
			// Else, look for a witness path
//			Edge newEdge;
			bool shortcut_necessary = contraction(e_in, n, e_out, true);//shortcutNeeded(e_in,n,e_out,true,newEdge);
			// If there is no witness path, a shortcut is needed, insertion indicators are updated
			if ( shortcut_necessary ){
				++nbInsEdge;
				nbOriginEdgeInsert += _graph->getBwEdge(e_in).getNbOriginalEdge() + _graph->getFwEdge(e_out).getNbOriginalEdge();
				complexityInsert += _graph->getBwEdge(e_in).getWeight().range() + _graph->getFwEdge(e_out).getWeight().range();
			}
		}
	}
	if( nbInsEdge > 0){
		// (REMOVAL INDICATORS) Loop over outcoming edges (that will be removed)
		for ( Edge_id e = _graph->getNodeBeginFW(n) ; e < _graph->getNodeEndFW(n) ; ++e ){
			++nbRemEdge;
			nbOriginEdgeRemove += _graph->getFwEdge(e).getNbOriginalEdge();
			complexityRemove += _graph->getFwEdge(e).getWeight().range();
		}
		// (REMOVAL INDICATORS) Loop over incoming edges (that will be removed)
		for ( Edge_id e = _graph->getNodeBeginBW(n) ; e < _graph->getNodeEndBW(n) ; ++e ){
			++nbRemEdge;
			nbOriginEdgeRemove += _graph->getBwEdge(e).getNbOriginalEdge();
			complexityRemove += _graph->getBwEdge(e).getWeight().range();
		}
	}
	// Calculation of edge quotient: relative comparison between numbers of inserted and removed edges
	double edges_quotient = double(nbInsEdge) / std::max(1.0, double(nbRemEdge));
	// Calculation of original edge quotient (how much original edges do the shortcuts/removed edges represent?)
	double original_edges_quotient = double(nbOriginEdgeInsert) / std::max(1.0, double(nbOriginEdgeRemove));
	// Calculation of complexity quotient (number of segments in TTF)
	double complexity_quotient = double(complexityInsert) / std::max(1.0, double(complexityRemove));
	// Priority coefficient fonction: 2 Edge quotient + Hierarchy depth + Original edge quotient + 2 Complexity quotient
	_contractionCost[n] = _config.getParamEq() * edges_quotient + _config.getParamSsd() * _nodeDepth[n] + _config.getParamOeq() * original_edges_quotient + _config.getParamCq() * complexity_quotient;
//	STATUS("Contraction cost of node N" << n << ": " << edges_quotient << "/" << _nodeDepth[n] << "/" << original_edges_quotient << "/" << complexity_quotient << " => " << _contractionCost[n] << "\n");
}

/*
 * shortcutNeeded(const Edge_id&, const Node_id&, const Edge_id&, const bool, Edge&) method: return the status of the shortcut (necessary or not necessary)
 * This method modify the edge passed as a parameter ('newEdge'), its ending value is the shortcut replacing backward edge 'e_in', node 'x' and forward edge 'e_out'
 */
bool Ordering::shortcutNeeded(const Edge_id& e_in, const Node_id& x, const Edge_id& e_out, const bool simulate, Edge& newEdge) {
	// Recover nodes u and v, knowing that path u->x->v is focused
	const Node_id u = _graph->getBwEdges()[e_in].getOrigin();
	const Node_id v =  _graph->getFwEdges()[e_out].getDestination();
	// Initialize witness and candidate shortcut
	WitnessCacheEntry witness;
	Distribution dist_ux = _graph->getBwEdges()[e_in].getWeight();
	Distribution dist_xv = _graph->getFwEdges()[e_out].getWeight();
	Distribution dist_uxv = dist_ux.convolute( dist_xv );
	uint32_t shortcutComplexity = (witness.getStatus() == WitnessSearch::UNDECIDED ? dist_uxv.range(): witness.getComplexity() );
	uint32_t nbOriginalEdge = _graph->getBwEdges()[e_in].getNbOriginalEdge() + _graph->getFwEdges()[e_out].getNbOriginalEdge();
	newEdge = Edge(true, u, v, dist_uxv, shortcutComplexity, nbOriginalEdge, x);
	// Use already cached witness if present
	if( !_wcache.empty( x ) ){
		witness = _wcache.lookup(u,x,v);
	}
	// Return if witness is already cached (Witness search status is either necessary or unnecessary)
	if ( witness.getStatus() != WitnessSearch::UNDECIDED ){
		return witness.getStatus() == WitnessSearch::NECESSARY;
	}
	// Here we don't know anything about shortcut necessity
	// Perform witness search
	if( _localThreads.size() == 0){
		_localThreads.push_back( LocalThread(_graph) );
	}
	LocalThread& locThread = _localThreads[ omp_get_thread_num() ] ;
	const uint32_t shortcut_status = locThread.run(u, x, v, dist_uxv); // Perform local search into the corresponding local thread
	// If it is a simulation (typically during the ordering initialization or when node costs are updated), save witness search result as a cache entry
	if ( simulate ){
		locThread.addCacheEntry( shortcut_status, shortcutComplexity, u, x, v );
	}
	return shortcut_status == WitnessSearch::NECESSARY;
}

/*
 * contract() method: update the graph in order to fasten the querying algorithms (highlights important nodes and add shortcut edges where that is needed)
 */
void Ordering::contract(const Edge_id& e_in, const Node_id x, const Edge_id& e_out, bool simulate){
	Edge candidateEdge;
	// First of all, decide if a shortcut is needed to represent the combination of e_in and e_out
	const bool shortcut_necessary = shortcutNeeded(e_in, x, e_out, simulate, candidateEdge);
	// If the shortcut is not necessary, stop the process
	if ( ! shortcut_necessary ){
		return;
	}
	// Here we know that a shortcut is needed, if its associated weight is an invalid distribution, recompute it (it shouldn't be the case: the weight should has been set during witness search)
	// The candidate edge has a new middle node (set as the deleted node, however it will have to be reset if shortcut must be merged with an existing edge)
	candidateEdge.setMiddleNode( x );
	// Add the shortcut to the local list of edges that will be added to the graph
	_localThreads[omp_get_thread_num()].addEdge( std::move(candidateEdge) );
}

/*
 * contraction(const Edge_id& , const Node_id , const Edge_id& , bool) method: determinate if a shortcut is needed to cover the deletion of provided node and edges, and save the shortcut in a set for further processing
 */
bool Ordering::contraction(const Edge_id& e_in, const Node_id x, const Edge_id& e_out, bool simulate){
	bool shortcutNeeded(false);
	// Recover nodes u and v, knowing that path u->x->v is focused
	const Node_id u = _graph->getBwEdges()[e_in].getOrigin();
	const Node_id v =  _graph->getFwEdges()[e_out].getDestination();
	Distribution dist_ux = _graph->getBwEdges()[e_in].getWeight();
	Distribution dist_xv = _graph->getFwEdges()[e_out].getWeight();
	Distribution dist_uxv = dist_ux.convolute( dist_xv );
//	TRACEF("Is a shortcut needed between N" << u << " and N" << v << " (through N" << x << ")? ");
	// Use already cached witness if present
	WitnessCacheEntry witness;
	if( !_wcache.empty( x ) ){
		witness = _wcache.lookup(u,x,v);
	}
	uint32_t shortcutComplexity = (witness.getStatus() == WitnessSearch::UNDECIDED ? dist_uxv.range(): witness.getComplexity() );
	uint32_t nbOriginalEdge = _graph->getBwEdges()[e_in].getNbOriginalEdge() + _graph->getFwEdges()[e_out].getNbOriginalEdge();
	// If witness is already cached (Witness search status is either necessary or unnecessary)
	if ( witness.getStatus() != WitnessSearch::UNDECIDED ){//&& witness.getStatus() != WitnessSearch::NOT_NECESSARY ){
		shortcutNeeded = witness.getStatus() == WitnessSearch::NECESSARY;
//		CONTINUE_STATUS( (shortcutNeeded?"YES":"NO") << " (cf cache)\n" );
	}
	else{
		// Otherwise we don't know anything about shortcut necessity
		// Perform witness search
		LocalThread& locThread = _localThreads[ omp_get_thread_num() ] ;
		const uint32_t shortcut_status = locThread.run(u, x, v, dist_uxv); // Perform local search into the corresponding local thread
		// If it is a simulation (typically during the ordering initialization or when node costs are updated), save witness search result as a cache entry
		if ( simulate ){
			locThread.addCacheEntry( shortcut_status, shortcutComplexity, u, x, v );
		}
		shortcutNeeded = shortcut_status == WitnessSearch::NECESSARY;
//		CONTINUE_STATUS( (shortcutNeeded?"YES":"NO") << "\n" );
	}
	// If the shortcut is not necessary, stop the process
	if ( shortcutNeeded && !simulate ){
		// Here we know that a shortcut is needed, build it according to the contracted node information
		Edge candidateEdge = Edge(true, u, v, dist_uxv, shortcutComplexity, nbOriginalEdge, x);
//		TRACE("Shortcut needed: N" << u << "->N" << x << "->N" << v);
		// Add the shortcut to the local list of edges that will be added to the graph
		_localThreads[ omp_get_thread_num() ].addEdge( std::move(candidateEdge) );
	}
	return shortcutNeeded;
}

/*
 * contractParallely() method: process to node contractions in a parallel manner (contract subset of independant nodes)
 */
void Ordering::contractParallely(){
	// First step: parallely contract in-processing nodes
#pragma omp parallel
	{
#pragma omp for schedule(dynamic) // Set-in parallel computing (on the working node set)
		for ( uint32_t i = _firstWorkingNode ; i < _lastWorkingNode ; ++i ){
			Node_id n(_nodeIds[i]);
			for ( Edge_id e_in = _graph->getNodeBeginBW(n) ; e_in != _graph->getNodeEndBW(n) ; ++e_in ){
				for ( Edge_id e_out = _graph->getNodeBeginFW(n) ; e_out != _graph->getNodeEndFW(n) ; ++e_out ){
					if ( _graph->getBwEdges()[e_in].getOrigin() != _graph->getFwEdges()[e_out].getDestination() ){
						contraction(e_in, n, e_out, false);//contract(e_in,n,e_out);
					}
				}
			}
		}
	}
	// Second step: update witness cache so as to keep only potential shortcuts (ie sequence u->x->v that can be reduce to u->v)
	for ( uint32_t i = _firstWorkingNode ; i < _lastWorkingNode ; ++i ){
		Node_id n(_nodeIds[i]);
		_wcache.remove( n ); // Remove all witness cache entries where n is the middle node
		// We still have to remove entries where n is the first or the third node of the sequence u->x->v
		for ( Edge_id e_in = _graph->getNodeBeginBW(n) ; e_in != _graph->getNodeEndBW(n) ; ++e_in ){
			Node_id u( _graph->getBwEdges()[e_in].getOrigin() );
			_wcache.remove(INVALID_NODE_ID, u, n); // Remove witness cache entries where n is the destination of the outcoming edge
			for ( Edge_id e_out = _graph->getNodeBeginFW(n) ; e_out != _graph->getNodeEndFW(n) ; ++e_out ){
				Node_id v( _graph->getFwEdges()[e_out].getDestination() );
				_wcache.remove(n, v, INVALID_NODE_ID); // Remove witness cache entries where n is the origin of the incoming edge
			}
		}
	}
}

/*
 * smallerCost(const Node_id&, const Node_id&) method: return true if node u has a smaller cost than node v, false otherwise
 * ties are broken with respect to node ids (in order to guarantee contraction stability during different instances)
 */
bool Ordering::smallerCost(const Node_id& u, const Node_id& v) const{
	if ( lt(_contractionCost[u],_contractionCost[v]) ){
		return true;
	}
	if ( eq(_contractionCost[u],_contractionCost[v]) && u < v){
		return true;
	}
	return false;
}

/*
 * localMinimum(const Node_id&, const int&) method: check if node x is the local minimum in terms of contraction cost in the graph
 * locality is defined through a hop number given as a parameter
 */
bool Ordering::localMinimum(const Node_id& x, const int& hop_radius) const{
	// Initialization : define a priority queue and insert node x inside (with value 0)
	std::queue<Node_id> Q;
	Q.push(x);
	std::unordered_map<uint32_t,int> n_hops;
	n_hops[x] = 0;
	// While there are nodes to visit, continue
	while ( ! Q.empty() ){
		// Consider the first node in the queue
		const Node_id u = Q.front();
		// Delete it...
		Q.pop();
		// ...and check the adjacent nodes: for each forward edges, evaluate destination node contraction cost
		for ( Edge_id e = _graph->getNodeBeginFW(u) ; e != _graph->getNodeEndFW(u) ; ++e ){
			const Node_id v = _graph->getFwEdge(e).getDestination();
			// If node v hop counter has already been set during a previous iteration, continue
			if ( n_hops.find(v) != n_hops.end() ){
				continue;
			}
			// If node v has a smaller contraction cost, x is not a local minimum
			if ( smallerCost(v, x) ){
				return false;
			}
			// Set node v hops (by increasing current node hop counter)
			n_hops[v] = n_hops[u] + 1;
			// If node v is too "far" from node x, do not insert it in the priority queue
			if ( n_hops[v] >= hop_radius){// || x == v){
				continue;
			}
			// Insert the node in the queue to relax it
			Q.push(v);
		}
		// Same process with backward edges
		for ( Edge_id e = _graph->getNodeBeginBW(u) ; e != _graph->getNodeEndBW(u) ; ++e ){
			const Node_id v = _graph->getBwEdge(e).getOrigin();
			// If node v hop counter has already been set during a previous iteration, continue
			if ( n_hops.find(v) != n_hops.end() ){
				continue;
			}
			// If node v has a smaller contraction cost, x is not a local minimum
			if ( smallerCost(v, x) ){
				return false;
			}
			// Set node v hops (by increasing current node hop counter)
			n_hops[v] = n_hops[u] + 1;
			// If node v is too "far" from node x, do not insert it in the priority queue
			if ( n_hops[v] >= hop_radius ){
				continue;
			}
			// Insert the node in the queue to relax it
			Q.push(v);
		}
	}
	return true;
}

/*
 * netContractionSet() method: choose nodes to be contracted next and move them in <code>_nodes</code>
 * such that they occur after <code>_working_nodes_begin</code>. This includes
 * setting up <code>_working_nodes_end</code>, such that
 * [_working_nodes_begin, _working_nodes_end)
 * contains the nodes to be contracted next afterwards.
 */
void Ordering::nextContractionSet(){
	// First step: chose nodes to be contracted next: they are local minima in terms of contraction cost
#pragma omp parallel
	{
#pragma omp for schedule(dynamic)
		for ( uint32_t i = _firstWorkingNode ; i < _nodeIds.size() ; ++i ){
			Node_id n(_nodeIds[i]);
			_nextNodeToContract[n] = localMinimum(n, 2);
		}
	}
	// Second step: move these nodes to the right position in <code>_nodeIds</code>, and update the index _lastWorkingNode
	auto first_unselected_node_it = std::stable_partition( _nodeIds.begin() + _lastWorkingNode , _nodeIds.end() , [&](const Node_id& x) { return _nextNodeToContract[x]; } );
	_lastWorkingNode = first_unselected_node_it - _nodeIds.begin();
}

/* ************************************************************************************************************************************************************** */
/*
 * run(int) method: proceed to hierarchy construction and graph contraction
 * master method of this class
 */
std::vector<Edge> Ordering::run(int nbThreads){
	HierarchyIO hierarchySaver();
	auto beginTime = time_stamp();
	// Initialize local thread vector with given number of threads (if the parameter is not specified, consider the maximum number of threads)
	_localThreads.clear();
	if ( nbThreads == -1 ){
		omp_set_num_threads( omp_get_max_threads() );
	}
	else{
		omp_set_num_threads( std::min(nbThreads, omp_get_num_procs()) );
	}
	_localThreads.reserve( omp_get_max_threads() );
	for ( int i = 0 ; i < omp_get_max_threads() ; ++i ){
		_localThreads.emplace_back( _graph );
	}
//	STATUS("Preprocessing running with " << std::min(nbThreads, omp_get_max_threads()) << " threads\n");
	// Parallely evaluate initial tentative node costs
//	STATUS("Computing initial node costs... (BEGINNING)\n");
	auto timer1 = time_stamp();
#pragma omp parallel
	{
#pragma omp for schedule(dynamic)
		for ( auto x = _nodeIds.begin() ; x < _nodeIds.end() ; ++x ){
			//			TRACE("Node to simulate:  N" << *x << "; fw edges: " << _graph->getNodeBeginFW(*x) << "-" << _graph->getNodeEndFW(*x) << "; bw edges = " << _graph->getNodeBeginBW(*x) << "-" << _graph->getNodeEndBW(*x) );
			simulateContraction( (Node_id)*x );
		}
	}
	// At this stage, we know the initial contraction cost of each node, and a first set of witness search has been accomplished
	// Update witness cache with local thread information
	for ( auto& thread_data : _localThreads ){
		for ( auto cacheEntry : thread_data.getCacheEntries() ){
			_wcache.insert(cacheEntry);
		}
		thread_data.clearCache();
	}
	//	STATUS("Witness cache after initialization:\n" << _wcache);
	auto timer2 = time_stamp();
	STATUS( "Total initialization time: " << get_duration_in_seconds(timer1, timer2) << " sec (" << omp_get_max_threads() << " threads)\n" );
	// End of the first step: initial costs are known, and witness cache is up-to-date
	// Repeatedly compute independent sets and contract their nodes, merging necessary shortcuts with already present edges
//	STATUS("Contract independent node sets...\n");
	/* Main While Loop ***********************************************************************************************************************************************************/
	while ( _firstWorkingNode < _nodeIds.size() ){
		// Select nodes that have to be contracted next, and sort the nodes so as to place them at the beginning of the node array (working node indices are subsequently set)
		nextContractionSet();
//		TRACE( "Contraction in processing: " << _firstWorkingNode << "-" << _lastWorkingNode - _firstWorkingNode << "-" << _nodeIds.size() - _lastWorkingNode << "\tContracted node(s): ");
		// Parallely contract these nodes and store needed shortcut (a shortcut is needed if the witness search fails to find a convenient witness path)
		contractParallely();
		// For each node that have been contracted:
		// - set its level
		// - identify the adjacent nodes (their depth must be updated immediately, as well as their contraction cost will be in the end of the current iteration)
		// - delete the node and all incident edges
		std::vector<Node_id> nodes_to_update;
		for ( size_t i = _firstWorkingNode ; i < _lastWorkingNode ; ++i ){
			Node_id x(_nodeIds[i]);
//			CONTINUE_STATUS("N" << x << " ");
			// Keep the level information
			_graph->setLevel(x,i);
			// Identify nodes that are adjacent by the way of forward edges and save corresponding outgoing edges
			for ( Edge_id e = _graph->getNodeBeginFW(x) ; e != _graph->getNodeEndFW(x) ; ++e ){
//				CONTINUE_STATUS("(" << _graph->getFwEdge(e).getOrigin() << "->" << _graph->getFwEdge(e).getDestination() << ") ");
				nodes_to_update.push_back( _graph->getFwEdge(e).getDestination() );
				_nodeDepth[_graph->getFwEdge(e).getDestination()] = std::max(_nodeDepth[_graph->getFwEdge(e).getDestination()], _nodeDepth[x] + 1);
				_newEdges.emplace_back( _graph->getFwEdge(e) );
			}
			// Identify nodes that are adjacent by the way of backward edges and save corresponding incoming edges
			for ( Edge_id e = _graph->getNodeBeginBW(x) ; e != _graph->getNodeEndBW(x) ; ++e ){
//				CONTINUE_STATUS("(" << _graph->getBwEdge(e).getOrigin() << "->" << _graph->getBwEdge(e).getDestination() << ") ");
				nodes_to_update.push_back( _graph->getBwEdge(e).getOrigin() );
				_nodeDepth[_graph->getBwEdge(e).getOrigin()] = std::max(_nodeDepth[_graph->getBwEdge(e).getOrigin()], _nodeDepth[x] + 1);
				_newEdges.emplace_back( _graph->getBwEdge(e) );
			}
			// Delete the node and all adjacent edges
			_graph->deleteNode(x);
//			CONTINUE_STATUS("\n");
		}
//		CONTINUE_STATUS("\n");
		// Remove eventual duplicated nodes (a node may be adjacent with several contracted nodes, and may consequently have been pushed in the vector several times)
		remove_duplicates(nodes_to_update);
//		TRACEF("New edges have been added: ");
		// Insert/merge new shortcut edges and remove merged edges from cache (otherwise computation will be wrong), for each local thread
		for ( auto& thread_data : _localThreads ){
			// For each stored candidate edge in the local thread structure (ie for each shortcut to save)
			for ( auto& candidateEdge : thread_data.getInsertedEdges() ){
				// Check if the edge is already physically in the graph (ie: Is there a physical link between the concerned origin and destination nodes?)
				Edge_id fwe = _graph->identifyFwEdge(candidateEdge.getOrigin(), candidateEdge.getDestination() );
				Edge_id bwe = _graph->identifyBwEdge(candidateEdge.getOrigin(), candidateEdge.getDestination() );
				/// If there still exists an edge in the graph between current origin and destination, remove out-of-date cache entries
				if ( fwe == INVALID_EDGE_ID ){
					// Create a new edge in the graph according to the shortcut information
//					Edge& shortcut( candidateEdge );
//					CONTINUE_STATUS("(" << candidateEdge.getOrigin() << "->" << candidateEdge.getDestination() << ")* ");
					_graph->addShortcut( std::move(candidateEdge) );
				}
				else{
					if( candidateEdge.getWeight().dominates( _graph->getFwEdge(fwe).getWeight() ) ){
						_graph->updateEdgeInfo( fwe , bwe , candidateEdge );
					}
					else{
//						CONTINUE_STATUS("(" << candidateEdge.getOrigin() << "->" << candidateEdge.getDestination() << ") ");
						_graph->addShortcut( std::move(candidateEdge) );
					}
					// Delete the corresponding cache entries (the edge distribution is now modified, previous shortcut evaluations are out-of-date)
					_wcache.remove(INVALID_NODE_ID, candidateEdge.getOrigin(), candidateEdge.getDestination());
					_wcache.remove(candidateEdge.getOrigin(), candidateEdge.getDestination(), INVALID_NODE_ID);
				}
			}
			thread_data.clearEdgeList();
		}
//		CONTINUE_STATUS("\n");
		// Parallely update contraction cost for stored adjacent nodes
#pragma omp parallel
		{
#pragma omp for schedule(dynamic)
			for ( auto x = nodes_to_update.begin() ; x < nodes_to_update.end() ; ++x ){
//				TRACE("Node to update:  N" << *x << "; fw edges: " << _graph->getNodeBeginFW(*x) << "-" << _graph->getNodeEndFW(*x) << "; bw edges = " << _graph->getNodeBeginBW(*x) << "-" << _graph->getNodeEndBW(*x) );
				simulateContraction( (Node_id)*x );
			}
		}
		// Update witness cache
		for ( auto& thread_data : _localThreads ){
			for ( auto cacheEntry : thread_data.getCacheEntries() ){
				_wcache.insert(cacheEntry);
			}
			thread_data.clearCache();
		}
		// Mark all nodes in the current independent set as contracted
		_firstWorkingNode = _lastWorkingNode;
	}
	/* End Main While Loop *******************************************************************************************************************************************************/
	auto endTime = time_stamp();
	double total_ordering_time = get_duration_in_seconds(beginTime, endTime);
	STATUS("Total ordering process took " << total_ordering_time << " sec (" << omp_get_max_threads() << " threads) in total.\n");
	return _newEdges;
}
/* ************************************************************************************************************************************************************** */

/*
 * serialize(std::string&) method: save node hierarchy and shortcuts into a text file
 * NOT YET IMPLEMENTED
 */
void Ordering::writeOrdering(const std::string& hierarchyfilename, const std::string& edgefilename){
	HierarchyIO hierarchySaver(hierarchyfilename, _graph->getSortedNodes(), _graph->getLevels() );
	hierarchySaver.write();
	EdgeIO shortcutSaver(edgefilename, _newEdges );
	shortcutSaver.write();
}

/*
 * << operator: send a short description of an Ordering instance into a output stream (printing purpose)
 * format:
 * nb_threads ; working_nodes (begin-end)
 * (1 line per node) (<Node_i>,<Level_of_node_i>,<Cost_i>,<Depth_i>)
 * Witness_cache
 */
std::ostream& operator<<(std::ostream& os, Ordering& order){
	TRACE("Ordering printing");
	std::vector<Node> graphnodes = order._graph->getNodes();
	std::vector<uint32_t> graphnodelevels = order._graph->getLevels();
	std::vector<uint32_t>::iterator itl = graphnodelevels.begin();
	std::vector<double>::iterator itc = order._contractionCost.begin();
	std::vector<uint32_t>::iterator itd = order._nodeDepth.begin();
	os << order._localThreads.size() << " thread(s) has(have) been used. Nodes currently in process: " << order._firstWorkingNode << "-" << order._lastWorkingNode << "\n";
	for(std::vector<Node>::iterator itn = graphnodes.begin() ; itn < graphnodes.end() ; ++itc, ++itd, ++itl, ++itn){
		os << "(" << itn - graphnodes.begin() << ";" << *itl << ";" << *itc << ";" << *itd << ")\n";
	}
	os << "" << order._wcache;
	return os;
}
