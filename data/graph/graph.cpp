/*
 * graph.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "graph.h"
#include "../../data_io/hierarchy_io.h"

Graph::Graph(){}
Graph::Graph(GraphReader graphdata, const Specif& specif): _specif(specif){
	std::vector<Edge> edges = graphdata.getEdges();
	/*
	 * Set-up nodes: affect ids between 0 and max_id (number of nodes)
	 */
	Node_id max_node_it(0);
	// For each edge in the list, determinate the max node id (the final value is the maximal node id in the graph)
	std::for_each( edges.begin(), edges.end(), [&max_node_it](const Edge& edges) -> void {
		max_node_it = std::max(max_node_it, edges.getOrigin());
		max_node_it = std::max(max_node_it, edges.getDestination());
	} );
	_nodes.assign(max_node_it + 1, Node()); // The max value is used to dimension the graph
	/*
	 * Set-up levels: by default, consider that node n°i is the i^th in the hierarchy (can be modified by the function ordering)
	 */
	_levels.assign(max_node_it + 1, INVALID_NODE_ID );
	std::iota(_levels.begin(),_levels.end(),0);
	_sortednodes.assign(max_node_it + 1, INVALID_NODE_ID );
	std::iota(_sortednodes.begin(),_sortednodes.end(),0);
	/*
	 * Set-up Forward edges
	 */
	std::sort( edges.begin(), edges.end(),[](const Edge& e1, const Edge& e2) -> bool {
		return (e1.getOrigin() != e2.getOrigin()) ? e1.getOrigin() < e2.getOrigin() : e1.getDestination() < e2.getDestination() ;
	} ); // Sort input edges by source id AND by destination id
	// For all edges, update the corresponding node description (if necessary) and update the forward edge list
	for ( auto it = edges.begin() ; it != edges.end() ; ++it ){
		if ( _nodes[it->getOrigin()].getBeginFW() == INVALID_EDGE_ID ){ // If the current edge is the first to leave the node it leaves...
			// If the current edge is not the first in the list, insert dummy edges to cover the further edge creations for previous node
			if ( it > edges.begin() ){
				uint32_t previous_source = (it-1)->getOrigin();
				uint32_t n_edges_of_previous_source = _nodes[previous_source].getEndFW() - _nodes[previous_source].getBeginFW();
				uint32_t n_dummies = std::max( size_t(n_edges_of_previous_source + 2), size_t(double(n_edges_of_previous_source) * GROWTH_FACTOR) ); // Reserve some space for further additional edges = max(#edges+2;1.2#edges)
				for ( uint32_t i = 0 ; i < n_dummies ; ++i ){
					_fwedges.push_back( std::move( Edge() ) );
				}
			}
			Edge_id fws( _fwedges.size() );
			_nodes[it->getOrigin()].setBeginFW( fws );
			_nodes[it->getOrigin()].setEndFW( fws );
		}
		// Add the edge in the forward edge list
		Distribution dist = it->getWeight();
		_fwedges.push_back( std::move( Edge( true , it->getOrigin() , it->getDestination() , dist , it->getMiddleNode() ) ) );
//		TRACE( "Last forward inserted edge: " << _fwedges.back() );
		// Update the node description (increment the forward edge counter)
		Edge_id fwedge_id(1 + _nodes[it->getOrigin()].getEndFW() );
		_nodes[it->getOrigin()].setEndFW( fwedge_id );
	}
	// If at least one edge exists, add dummy edges to complete the vector (in case of further edge creation) for the last studied node
	if ( ! edges.empty() ){
		uint32_t previous_source = edges.back().getOrigin();
		uint32_t n_edges_of_previous_source = _nodes[previous_source].getEndFW() - _nodes[previous_source].getBeginFW();
		uint32_t n_dummies = std::max( uint32_t(n_edges_of_previous_source + 2), uint32_t(double(n_edges_of_previous_source) * GROWTH_FACTOR) ); // Reserve some space for further additional edges = max(#edges+2;1.2#edges)
		for ( uint32_t i = 0 ; i < n_dummies ; ++i ){
			_fwedges.push_back( std::move( Edge() ) );
		}
	}
	// Edges have been saved, the initial structure can be emptied
	edges.clear();
	/*
	 * Set-up Backward edges
	 */
	std::vector<Edge> backward_edge_list;
	backward_edge_list.reserve( max_node_it );
	// For each forward edge of each node, generate an edge aiming at modeling the corresponding backward edge
	Node_id maxnodeid( _nodes.size() );
	for ( Node_id source(0) ; source != maxnodeid ; ++source ){
		for ( Edge_id fw_edge_it = _nodes[source].getBeginFW() ; fw_edge_it < _nodes[source].getEndFW() ; ++fw_edge_it ){
			assert( fw_edge_it < _fwedges.size() );
			Distribution dist = _fwedges[fw_edge_it].getWeight();
			Edge curEdge = Edge( false , source , _fwedges[fw_edge_it].getDestination() , fw_edge_it , dist , _fwedges[fw_edge_it].getMiddleNode() );
			backward_edge_list.push_back( curEdge );
		}
	}
	// Sort the new edges according to the destination ids
	std::sort(backward_edge_list.begin(), backward_edge_list.end(),
			[](const Edge& e1, const Edge& e2) -> bool { return e1.getDestination() < e2.getDestination(); }
	);
	// For each new edge, create a final version that will compose the graph
	for ( auto it = backward_edge_list.begin() ; it != backward_edge_list.end() ; ++it ){
		//			assert( it->_target < _nodes.size() );
		if ( _nodes[it->getDestination()].getBeginBW() == INVALID_EDGE_ID ){
			// If the current edge is not the first in the list, insert dummy edges to cover the further edge creations
			if ( it > backward_edge_list.begin() ){
				uint32_t previous_target = (it-1)->getDestination();
				uint32_t n_edges_of_previos_target = _nodes[previous_target].getEndBW() - _nodes[previous_target].getBeginBW();
				uint32_t n_dummies = std::max( uint32_t(n_edges_of_previos_target + 2), uint32_t(double(n_edges_of_previos_target) * GROWTH_FACTOR) ); // Reserve some space for further additional edges = max(#edges+2;1.2#edges)
				_bwedges.insert(_bwedges.end(), n_dummies, Edge() );
			}
			Edge_id bws( _bwedges.size() );
			_nodes[it->getDestination()].setBeginBW( bws );
			_nodes[it->getDestination()].setEndBW( bws );
		}
		// Set the symmetric edge id according to the size of the backward edge vector
		Edge_id bwsym( _bwedges.size() );
		_fwedges[it->getSymEdge()].setSymEdge( bwsym );
		// Add the edge in the backward edge list (the backward edge id can't be larger than the forward edge max id, as both sets are symmetric)
		Distribution dist = it->getWeight();
		_bwedges.push_back( Edge( false , it->getOrigin() , it->getDestination() , it->getSymEdge() , dist , it->getMiddleNode() ) );
		//		TRACE( "Last backward inserted edge: " << _bwedges.back() );
		// Update the node description (increment the backward edge counter)
		Edge_id bwedge_id(1 + _nodes[it->getDestination()].getEndBW() );
		_nodes[it->getDestination()].setEndBW( bwedge_id );
	}
	// If some edges have been created, add dummy edges to complete the vector (in case of further edge creation)
	if ( ! backward_edge_list.empty() ){
		uint32_t previous_target = backward_edge_list.back().getDestination();
		uint32_t n_edges_of_previous_source = _nodes[previous_target].getEndBW() - _nodes[previous_target].getBeginBW();
		uint32_t n_dummies = std::max( uint32_t(n_edges_of_previous_source + 2), uint32_t(double(n_edges_of_previous_source) * GROWTH_FACTOR) ); // Reserve some space for further additional edges = max(#edges+2;1.2#edges)
		for ( uint32_t i = 0 ; i < n_dummies ; ++i ){
			_bwedges.push_back( Edge() );
		}
	}
	// Edges have been saved, the initial structure can be emptied
	backward_edge_list.clear();
}

/*
 * Destructor
 */
Graph::~Graph(){
	_nodes.clear();
	_fwedges.clear();
	_bwedges.clear();
}

/*
 * Getters
 */
Specif Graph::getSpecif() const{ return _specif; }
uint32_t Graph::getNbNodes() const{ return _specif.getNbNodes(); }
uint32_t Graph::getNbEdges() const{ return _specif.getNbEdges(); }
uint32_t Graph::getNbPtss() const{ return _specif.getNbPts(); }
uint32_t Graph::getDelta() const{ return _specif.getDelta(); }
std::vector<Node> Graph::getNodes() const{ return _nodes; }
Node Graph::getNode(const Node_id& n) const { return _nodes[n]; }
std::vector<Edge> Graph::getFwEdges() const{ return _fwedges; }
Edge Graph::getFwEdge(const Edge_id& e) const { return _fwedges[e]; }
std::vector<Edge> Graph::getBwEdges() const{ return _bwedges; }
Edge Graph::getBwEdge(const Edge_id& e) const { return _bwedges[e]; }
std::vector<Node_id> Graph::getSortedNodes() const{ return _sortednodes; }
Node_id Graph::getSortedNode(const uint32_t& index) const{ return _sortednodes[index]; }
std::vector<uint32_t> Graph::getLevels() const{ return _levels; }
uint32_t Graph::getLevel(const uint32_t& index) const{ return _levels[index]; }
Edge_id Graph::getNodeBeginBW(Node_id n) const{ return _nodes[n].getBeginBW(); }
Edge_id Graph::getNodeEndBW(Node_id n) const{ return _nodes[n].getEndBW(); }
Edge_id Graph::getNodeBeginFW(Node_id n) const{ return _nodes[n].getBeginFW(); }
Edge_id Graph::getNodeEndFW(Node_id n) const{ return _nodes[n].getEndFW(); }

/*
 * Setters
 */
void Graph::setLevel(const Node_id& n, const uint32_t& l){ /*TRACE("Node N" << n << " has level L" << l);*/ _levels[n] = l; _sortednodes[l] = n; }
void Graph::setFwEdgeWeight(const Edge_id& edge_id, Distribution& dist){ _fwedges[edge_id].setWeight( std::move(dist) ); }
void Graph::setBwEdgeWeight(const Edge_id& edge_id, Distribution& dist){ _bwedges[edge_id].setWeight( std::move(dist) ); }
void Graph::setNodeBeginBW(const Node_id tgt, const Edge_id& e){ _nodes[tgt].setBeginBW(e); }
void Graph::setNodeEndBW(const Node_id tgt, const Edge_id& e){ _nodes[tgt].setEndBW(e); }
void Graph::setNodeBeginFW(const Node_id src, const Edge_id& e){ _nodes[src].setBeginFW(e); }
void Graph::setNodeEndFW(const Node_id src, const Edge_id& e){ _nodes[src].setEndFW(e); }
void Graph::setHierarchy(const std::string& hierarchyFileName){
	HierarchyIO hierarchyReader = HierarchyIO(hierarchyFileName, _sortednodes, _levels);
	hierarchyReader.read();
	hierarchyReader.recoverHierarchy(_sortednodes,_levels);
//	STATUS("Hierarchy recovered!\n");
}

/*
 * aggregateEdge(const Edge_id&, const Edge&) method: update forward&backward edges at given index with candidate edge (info: distribution + middle node)
 */
void Graph::updateEdgeInfo(const Edge_id& fwe, const Edge_id& bwe, const Edge& candidateEdge){
	_fwedges[fwe].aggregate( candidateEdge );
	_fwedges[fwe].setMiddleNode( candidateEdge.getMiddleNode() );
	_bwedges[bwe].aggregate( candidateEdge );
	_bwedges[bwe].setMiddleNode( candidateEdge.getMiddleNode() );

}

/*
 * aggregateFwEdge(const Edge_id&, const Edge&) method: aggregate forward edge at given index with candidate edge
 */
void Graph::aggregateFwEdge(const Edge_id& e, const Edge& candidateEdge){ _fwedges[e].aggregate( candidateEdge ); }

/*
 * aggregateBwEdge(const Edge_id&, const Edge&) method: aggregate backward edge at given index with candidate edge
 */
void Graph::aggregateBwEdge(const Edge_id& e, const Edge& candidateEdge){ _bwedges[e].aggregate( candidateEdge ); }

/*
 * addShortcut(const Edge&) method: add a new shortcut edge to the graph
 */
void Graph::addShortcut(Edge&& edge){
	Edge_id fwInsert = identifyFwInsertId( edge.getOrigin() );
	Edge_id bwInsert = identifyBwInsertId( edge.getDestination() );
	_fwedges[fwInsert] = edge;
	_fwedges[fwInsert].setSymEdge( bwInsert );
	_bwedges[bwInsert] = std::move(edge);
	_bwedges[bwInsert].setDirection( false );
	_bwedges[bwInsert].setSymEdge( fwInsert );
	_specif.incrementEdge();
}

/*
 * identifyFwInsertId(const std::vector<Edge>&, Node_id, Node_id) method: return id at which an edge can be inserted
 */
Edge_id Graph::identifyFwInsertId( const Node_id& node ){
	Edge_id begin( getNodeBeginFW(node) );
	Edge_id end( getNodeEndFW(node) );
	Edge_id insertId(INVALID_EDGE_ID);
//	TRACE("Insert edge in the set " << begin << "-" << end);
	// Case 1: unconnected node, last edge id is not initialized yet => create some dummy edges to prepare more edge insertions
	if( end == INVALID_EDGE_ID ){
//		TRACE("[" << begin << "-" << end << "] CASE 1: end id is invalid");
		const uint32_t oldNbEdges = 0;
		const uint32_t nbDummies = 1 + std::max( oldNbEdges+3, uint32_t((1+oldNbEdges)*GROWTH_FACTOR) ) - (1+oldNbEdges);
		for ( uint32_t i = 0 ; i < nbDummies ; ++i ){
			_fwedges.emplace_back();
		}
		// Begin and end id are respectively the edge container size and this size + 1
		setNodeBeginFW( node , Edge_id(_fwedges.size()) );
		insertId = _fwedges.size();
		setNodeEndFW( node , Edge_id(_fwedges.size() + 1) );
	}
	// Case 2: end id corresponds to edge container size => more dummy edges are needed to prepare shortcut insertions
	else if( end == _fwedges.size() ){
//		TRACE("[" << begin << "-" << end << "] CASE 2: end id is the last edge");
		const uint32_t oldNbEdges = end - begin;
		const uint32_t nbDummies = 1 + std::max( oldNbEdges+3, uint32_t((1+oldNbEdges)*GROWTH_FACTOR) ) - (1+oldNbEdges);
		for ( uint32_t i = 0 ; i < nbDummies ; ++i ){
			_fwedges.emplace_back();
		}
		// Final insert id is the old container size, the end id is subsequently incremented
		insertId = end;
		setNodeEndFW( node , Edge_id(end + 1) );
	}
	// Case 3: origin node of the end edge is invalid
	else if( _fwedges[end].getOrigin() == INVALID_NODE_ID ){
//		TRACE("[" << begin << "-" << end << "] CASE 3: origin node of the end edge is invalid => add an edge at the next free index");
		insertId = end;
//		Edge_id newEndFW(end+1);
		setNodeEndFW( node , Edge_id(end + 1) );
	}
	// Case 4: origin node of the edge before begin is invalid
	else if( begin > 0 && _fwedges[begin - 1].getOrigin() == INVALID_NODE_ID ){
//		TRACE("[" << begin << "-" << end << "] CASE 4: begin is strictly positive, and the origin node of begin-1 edge is invalid => add an edge at the last free index");
		insertId = begin-1;
		setNodeBeginFW( node , Edge_id(begin-1) );
	}
	// Case 5:
	else{
//		TRACE("[" << begin << "-" << end << "] CASE 5: other cases => no available location before or after edge sequence");
		const uint32_t oldSize( _fwedges.size() );
		const uint32_t oldNbEdges = end - begin;
		// Move existing forward edges to the end of the adjacency array and make the previous locations dummy
		for ( Edge_id it = begin ; it != end ; ++it ){
			Edge_id nbEdges( _fwedges.size() );
			_bwedges[ _fwedges[it].getSymEdge() ].setSymEdge( nbEdges );
			_fwedges.emplace_back( std::move(_fwedges[it]) );
			_fwedges[it].makeDummy();
		}
		// The new 'begin' and 'end' indices are set as oldsize and newsize+1
		setNodeBeginFW( node , Edge_id(oldSize) );
		insertId = _fwedges.size();
		setNodeEndFW( node , Edge_id(_fwedges.size() + 1) );
		// dummy edges are added at the end of the edge container to prepare some further edge insertions
		const uint32_t nbDummies = 1 + std::max( oldNbEdges+3, uint32_t((1+oldNbEdges)*GROWTH_FACTOR) ) - (1+oldNbEdges);
		for ( uint32_t i = 0 ; i < nbDummies ; ++i ){
			_fwedges.emplace_back();
		}
	}
//	TRACE("Insert a new FW edge at " << insertId);
	return insertId;
}

/*
 * identifyBwInsertId(const std::vector<Edge>&, Node_id, Node_id) method: return id at which an edge can be inserted
 */
Edge_id Graph::identifyBwInsertId( const Node_id& node ){
	Edge_id begin( getNodeBeginBW(node) );
	Edge_id end( getNodeEndBW(node) );
//	TRACE("Insert edge in the set " << begin << "-" << end);
	Edge_id insertId(INVALID_EDGE_ID);
	// Case 1:
	if( end == INVALID_EDGE_ID ){
//		TRACE("[" << begin << "-" << end << "] CASE 1: end id is invalid");
		const uint32_t oldNbEdges = 0;
		const uint32_t nbDummies = 1 + std::max( oldNbEdges+3, uint32_t((1+oldNbEdges)*GROWTH_FACTOR) ) - (1+oldNbEdges);
		for ( uint32_t i = 0 ; i < nbDummies ; ++i ){
			_bwedges.emplace_back();
		}
		setNodeBeginFW( node , Edge_id(_bwedges.size()) );
		insertId = _bwedges.size();
		setNodeEndFW( node , Edge_id(_bwedges.size() + 1) );
	}
	// Case 2:
	else if( end == _bwedges.size() ){
//		TRACE("[" << begin << "-" << end << "] CASE 2: end id is the last edge");
		const uint32_t oldNbEdges = end - begin;
		const uint32_t nbDummies = 1 + std::max( oldNbEdges+3, uint32_t((1+oldNbEdges)*GROWTH_FACTOR) ) - (1+oldNbEdges);
		for ( uint32_t i = 0 ; i < nbDummies ; ++i ){
			_bwedges.emplace_back();
		}
		insertId = end;
		setNodeEndBW( node , Edge_id(end + 1) );
	}
	// Case 3:
	else if( _bwedges[end].getOrigin() == INVALID_NODE_ID ){
//		TRACE("[" << begin << "-" << end << "] CASE 3: origin node of the end edge is invalid => add an edge at the next free index");
		insertId = end;
		setNodeEndBW( node , Edge_id(end + 1) );
	}
	// Case 4:
	else if( begin > 0 && _bwedges[begin - 1].getOrigin() == INVALID_NODE_ID ){
//		TRACE("[" << begin << "-" << end << "] CASE 4: begin is strictly positive, and the origin node of begin-1 edge is invalid");
		setNodeBeginBW( node , Edge_id(begin-1) );
		insertId = begin-1;
	}
	// Case 5:
	else{
//		TRACE("[" << begin << "-" << end << "] CASE 5: other cases");
		const uint32_t oldSize( _bwedges.size() );
		const uint32_t oldNbEdges = end - begin;
		for ( Edge_id it = begin ; it != end ; ++it ){
			Edge_id nbEdges( _bwedges.size() );
			_fwedges[ _bwedges[it].getSymEdge() ].setSymEdge( nbEdges );
			_bwedges.emplace_back( std::move(_bwedges[it]) );
			_bwedges[it].makeDummy();
		}
		setNodeBeginBW( node , Edge_id(oldSize) );
		insertId = _bwedges.size();
		setNodeEndBW( node , Edge_id(_bwedges.size() + 1) );
		const uint32_t nbDummies = 1 + std::max( oldNbEdges+3, uint32_t((1+oldNbEdges)*GROWTH_FACTOR) ) - (1+oldNbEdges);
		for ( uint32_t i = 0 ; i < nbDummies ; ++i ){
			_bwedges.emplace_back();
		}
	}
//	TRACE("Insert a new BW edge at " << insertId);
	return insertId;
}

/*
 * identifyFwEdge(const Node_id&, const Node_id&) method: if there is a forward edge between given source and destination, return its id (INVALID_EDGE_ID otherwise)
 */
Edge_id Graph::identifyFwEdge(const Node_id& origin, const Node_id& destination){
	for ( Edge_id e = getNodeBeginFW(origin) ; e != getNodeEndFW(origin) ; ++e ){
		if ( _fwedges[e].getDestination() == destination ){
			return e;
		}
	}
	return INVALID_EDGE_ID;
}

/*
 * identifyBwEdge(const Node_id&, const Node_id&) method: if there is a forward edge between given source and destination, return its id (INVALID_EDGE_ID otherwise)
 */
Edge_id Graph::identifyBwEdge(const Node_id& origin, const Node_id& destination){
	for ( Edge_id e = getNodeBeginBW(destination) ; e != getNodeEndBW(destination) ; ++e ){
		if ( _bwedges[e].getOrigin() == origin ){
			return e;
		}
	}
	return INVALID_EDGE_ID;
}

/*
 * deleteNode(const Node_id&) method: delete a node from the graph, and all subsequent adjacent edges
 */
void Graph::deleteNode(const Node_id& u){
//	TRACE("Delete node N" << u << ": " << _nodes[u]);
//	TRACE("Node N" << u << ": bw edges = " << getNodeBeginBW(u) << "-" << getNodeEndBW(u) << "; fw edges: " << getNodeBeginFW(u) << "-" << getNodeEndFW(u));
	int nbDeletedNodes(1), nbDeletedEdges(0);
	// Update graph specification
	nbDeletedEdges += (getNodeEndFW(u) - getNodeBeginFW(u)) + (getNodeEndBW(u) - getNodeBeginBW(u));
	_specif.setNbNodes( _specif.getNbNodes() - nbDeletedNodes );
	_specif.setNbEdges( _specif.getNbEdges() - nbDeletedEdges );
	// Remove outgoing edges and their corresponding backward edges (begin by last ones, to avoid a permutation)
	for ( Edge_id e_fw = getNodeBeginFW(u) ; e_fw != getNodeEndFW(u)  ; ++e_fw ) {
		Node_id tgt = getFwEdges()[e_fw].getDestination();
		Edge_id lastTargetEdge( _nodes[tgt].getEndBW() - 1 );
		// If the symmetric edge is not the last one of the target node, shift it with the last one (to keep a coherent adjacency array, with dummy edges stored at the end)
		Edge_id symEdge = getFwEdges()[e_fw].getSymEdge();
		if ( symEdge != lastTargetEdge ){
			_bwedges[symEdge] = _bwedges[lastTargetEdge];
			_fwedges[_bwedges[symEdge].getSymEdge()].setSymEdge(symEdge);
		}
		// Delete the last target edge (corresponding to the symmetric edge of current one)
		_bwedges[lastTargetEdge].makeDummy();
		// Decrease bw edge counter for target node
		_nodes[tgt].decreaseEndBW( );
		// Delete the current edge
		_fwedges[e_fw].makeDummy();
	}
	_nodes[u].setEndFW( getNodeBeginFW(u) );
	// Remove incoming edges and their corresponding forward edges
	for ( Edge_id e_bw = getNodeBeginBW(u) ; e_bw != getNodeEndBW(u) ; ++e_bw ){
		Node_id src = _bwedges[e_bw].getOrigin();
		Edge_id lastSourceEdge( _nodes[src].getEndFW() - 1 );
		// If the symmetric edge is not the last one of the target node, shift it with the last one (to keep a coherent adjacency array, with dummy edges stored at the end)
		Edge_id symEdge = getBwEdges()[e_bw].getSymEdge();
		if ( symEdge != lastSourceEdge ){
			_fwedges[symEdge] = std::move( _fwedges[lastSourceEdge] );
			_bwedges[_fwedges[symEdge].getSymEdge()].setSymEdge(symEdge);
		}
		// Delete the last source edge (corresponding to the symmetric edge of current one)
		_fwedges[lastSourceEdge].makeDummy();
		// Decrease fw edge counter for source node
		_nodes[src].decreaseEndFW( );
		// Delete the current edge
		_bwedges[e_bw].makeDummy();
	}
	// Set the current node indices so as begin and end iterators are equal (this node do not have any adjacent edge any more)
	_nodes[u].setEndBW( getNodeBeginBW(u) );
//	TRACE("Node N" << u << ": bw edges = " << getNodeBeginBW(u) << "-" << getNodeEndBW(u) << "; fw edges: " << getNodeBeginFW(u) << "-" << getNodeEndFW(u));
}

/*
 * reset() method: reset the graph (goes back to initial state before node hierarchy building)
 * reset the node levels (they are set as the node ids) and delete the shortcuts
 */
void Graph::reset(){
	std::iota(_levels.begin(), _levels.end(), 0);
	std::iota(_sortednodes.begin(), _sortednodes.end(), 0);
}

/*
 * << operator: return an outstream version of the graph (printing purpose)
 * format (line 1): <Specif>
 * format (line 2->#nodes+1): {Ei0bw,...,Eikbw} -> Ni -> {Ei0fw,...,Eikfw}, where Eijbw is the edge linking nodes j and i (in a backward manner)
 * format (line (#nodes+2)->#nodes+#edges+1) Ei: {(t_0,p_0,F_0) ... (t_k(i),p_k(i),F_k(i))}, where t, p and F are respectively the sets of support points, pmf and cdf // ie Ei: {<Distribution>}
 */
std::ostream& operator<<(std::ostream& os, Graph& graph){
	os << "***** Graph printing... *****\n";
	// Part 1: Specif printing
	os << graph._specif;
	// Part 2: Node printing
	TRACE("Node printing...");
	for(std::vector<Node>::iterator itn = graph._nodes.begin() ; itn < graph._nodes.end() ; ++itn){
		Edge_id curedge_id(0);
		if( itn->getBeginBW() < itn->getEndBW() ){
			os << "[BW]{E" << itn->getBeginBW();
			for(curedge_id = itn->getBeginBW()+1 ; curedge_id < itn->getEndBW() ; ++curedge_id){
				os << "," << "E" << curedge_id;
			}
			os << "}";
		}
		os << " -> N" << itn-graph._nodes.begin() << " -> ";
		if( itn->getBeginFW() < itn->getEndFW() ){
			os  << "[FW]{E" << itn->getBeginFW();
			for(curedge_id = itn->getBeginFW()+1 ; curedge_id < itn->getEndFW() ; ++curedge_id){
				os << "," << "E" << curedge_id;
			}
			os << "}";
		}
		os << "\n";
	}
	// Part 3: Edges printing
	TRACE("Backward edge printing... (" << graph._bwedges.end() - graph._bwedges.begin() << " edges)");
	std::vector<Edge>::iterator ite = graph._bwedges.begin(), itebeg = graph._bwedges.begin();
	for( ; ite < graph._bwedges.end() ; ++ite){
		os << "E" << ite - itebeg << "[BW]<E" << ite->getSymEdge() << "[FW]> {";
		os << *ite;
		os << "}\n";
	}
	TRACE("Forward edge printing... (" << graph._fwedges.end() - graph._fwedges.begin() << " edges)");
	for(ite = graph._fwedges.begin() ; ite < graph._fwedges.end() ; ++ite){
		os << "E" << ite - graph._fwedges.begin() << "[FW]<E" << ite->getSymEdge() << "[BW]> {";
		os << *ite;
		os << "}\n";
	}
	// Part 3: Hierarchy printing
	TRACE("Hierarchy printing...");
	os << "{ ";
	for( std::vector<uint32_t>::iterator itsn = graph._levels.begin() ; itsn < graph._levels.end() ; ++itsn){
		os << "[N" << itsn - graph._levels.begin() << " - L" << *itsn << "] ";
	}
	os << "}\n";
	os << "{ ";
	for( std::vector<Node_id>::iterator itsn = graph._sortednodes.begin() ; itsn < graph._sortednodes.end() ; ++itsn){
		os << "[L" << itsn - graph._sortednodes.begin() << " - N" << *itsn << "] ";
	}
	os << "}\n";
	os << "*****************************\n";
	return os;
}
