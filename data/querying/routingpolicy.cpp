/*
 * routingPolicy.cpp
 *
 *  Created on: 9 mars 2016
 *      Author: delhome
 */

#include "routingpolicy.h"

/*
 * Constructors
 */
RoutingPolicy::RoutingPolicy(){}
RoutingPolicy::RoutingPolicy(const Node_id& n, const int& size, const uint32_t& delta, const bool& empty): _node(n){
	if(empty){
		_frontier = Distribution(size,delta);
		_bestpaths.assign(size,std::numeric_limits<uint32_t>::max());
	}
	else{
		_paths[1] = Path(1,n,size,delta);
		_frontier = _paths[1].getDistribution();
		_bestpaths.assign( _frontier.getSize() , 1 );
	}
}
RoutingPolicy::RoutingPolicy(const Node_id& n, const std::map<uint32_t,Path>& paths, const Distribution& dist, const std::vector<uint32_t>& bp): _node(n){
	_paths = paths;
	_frontier = dist;
	_bestpaths = bp;
}
RoutingPolicy::RoutingPolicy(const RoutingPolicy& rp, const Node_id& n, const Edge_id& e, const Distribution& dist): _node( n ){
	for(auto& path: rp.getPaths()){
		_paths[ path.first ] = Path(path.second,n,e,dist);
	}
	_frontier = rp.getFrontier().convolute(dist);
	_bestpaths = rp.getBestPathIds();
}

/*
 * Getters
 */
Node_id RoutingPolicy::getNode() const{ return _node; }
Distribution RoutingPolicy::getFrontier() const{ return _frontier; }
double RoutingPolicy::getShortestTime() const { return _frontier.min(); }
double RoutingPolicy::getLargestTime() const { return _frontier.max(); }
std::array<double,3> RoutingPolicy::getFrontierT(const uint32_t& index) const{
	std::array<double,3> frontierElement;
	frontierElement[0] = _frontier.getSupT(index);
	frontierElement[1] = _frontier.getPdfT(index);
	frontierElement[2] = _frontier.getCdfT(index);
	return frontierElement;
}
std::map<uint32_t,Path> RoutingPolicy::getPaths() const { return _paths; }
std::vector<uint32_t> RoutingPolicy::getBestPathIds() const { return _bestpaths; }
uint32_t RoutingPolicy::getBestPathId(const uint32_t& index) const { return _bestpaths[index]; }
uint32_t RoutingPolicy::getNbPaths() const{ return _paths.size(); }

/*
 * Setters
 */
void RoutingPolicy::setNode(const Node_id& n){ _node = n; }
void RoutingPolicy::setFrontier(const Distribution& dist){ _frontier = dist; }
void RoutingPolicy::setFrontierT(const uint32_t& index, const double& pdfValue, const double& cdfValue){
	_frontier.setPdfT(index,pdfValue);
	_frontier.setCdfT(index,cdfValue);
}
void RoutingPolicy::getBestPathIds(const std::vector<uint32_t>& bp){
	_bestpaths = bp;
}

/*
 * empty() method: return true if there is not any path is the policy (the distribution must be infinite in this case), false otherwise
 */
bool RoutingPolicy::empty(){
	if( _paths.size() == 0 ){
		assert( _frontier.isInfinite() );
		return true;
	}
	return false;
}

/*
 * clear() method: delete the saved paths and reset the frontier as an infinite distribution
 */
void RoutingPolicy::clear(){
	_paths.clear();
	_frontier.makeInfinite();
}

/*
 * reset(const RoutingPolicy&, const Edge_id&, const Distribution&) method: reset the routing policy and fill it with a new path
 */
void RoutingPolicy::reset(const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist){
	_paths.clear();
	_frontier.reset();
	_bestpaths.clear();
	for(auto& path: rp.getPaths()){
		_paths[ path.first ] = Path(path.second,_node,e,dist);
	}
	_frontier = rp.getFrontier().convolute(dist);
	_bestpaths = rp.getBestPathIds();
}

/*
 * addPath(SpotarPath&) method: add the path in the candidate set if it is local-reliable
 * return true if the path is really local-reliable, false otherwise
 */
bool RoutingPolicy::addPath(Path& candidatepath, const uint32_t& candidateid){
	//	TRACE("Check if following path is local-reliable: " << candidatepath);
	bool localreliable(false);
	if( _paths.size() == 0 ){
		localreliable = true;
		_paths[1] = candidatepath;
		_paths[1].setDSD(_frontier.getSize());
		_frontier = _paths[1].getDistribution();
		_bestpaths.assign( _frontier.getSize() , 1 );
		return localreliable;
	}
	// Here we know that there are still existing LR paths
	// Compare the cdf values between candidate path and current best path, for each time budget
	for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
		//		TRACE("t = " << t*_frontier.getDelta() << ": " << _frontier.getCdfT(t) << " vs. " << candidatepath.getDistribT(t)[2] << "\t");
		uint32_t lastbestpathid( _bestpaths[t] );
		// If the candidate is strictly larger than the current best path at the t index
		if( lt( _frontier.getCdfT(t) , candidatepath.getDistribT(t)[2] ) ){
			//			CONTINUE_STATUS("Decrement DSD of path P" << _bestpaths[t] << " at index " << t << " ; increment those of P" << candidateid << "\n");
			// Set the frontier cdf
			_frontier.setCdfT( t , candidatepath.getDistribT(t)[2] );
			// Decrement degree of strong dominance for defeated path, and increment those of candidate path
			_paths[lastbestpathid].decrementDSD();
			candidatepath.incrementDSD();
			// Set the best path for segment t as the candidate path id
			_bestpaths[t] = candidateid;
			if( _paths[lastbestpathid].getDSD() == 0 ){
				_paths.erase(lastbestpathid);
			}
		}
		// Do not forget that pdf must be updated accordingly!
		if(t == 0){
			//			if(!(le(0,_frontier.getCdfT(t))&&le(_frontier.getCdfT(t),1))){
			//				ERROR("Invalid pdf value at index t= " << t << "... " << _frontier.getCdfT(t) << " (old frontier pdf: " << _frontier.getPdfT(t) << ")\n");
			//			}
			_frontier.setPdfT( t , _frontier.getCdfT(t) );
		}else{
			//			if(!(le(0,_frontier.getCdfT(t)-_frontier.getCdfT(t-1))&&le(_frontier.getCdfT(t)-_frontier.getCdfT(t-1),1))){
			//				ERROR("Invalid pdf value at index t= " << t << "... " << _frontier.getCdfT(t)-_frontier.getCdfT(t-1) << " (old frontier pdf: " << _frontier.getPdfT(t) << ")\n");
			//			}
			_frontier.setPdfT( t , _frontier.getCdfT(t)-_frontier.getCdfT(t-1) );
		}
	}
	//		TRACE("Initial Cdf comparison: OK");
	// If candidate path has a strictly positive degree of strong dominance, it must be added to the path container (at this point, it should be true)
	if( candidatepath.getDSD() > 0 ){
		localreliable = true;
		_paths[ candidateid ] = candidatepath;
	}
	return localreliable;
}

/*
 * addPathClean(SpotarPath&) method: add the path in the candidate set if it is local-reliable and update this set accordingly by erasing obsolete paths
 * return true if the path is really local-reliable, false otherwise
 */
bool RoutingPolicy::addPathClean(Path& candidatepath, const uint32_t& candidateid){
	TRACE("Check if following path is local-reliable: " << candidatepath);
	bool localreliable(false);
	if( _paths.size() == 0 ){
		_paths[1] = candidatepath;
		_paths[1].setDSD(_frontier.getSize());
		_frontier = _paths[1].getDistribution();
		_bestpaths.assign( _frontier.getSize() , 1 );
		return localreliable;
	}
	else{
		// There are still existing LR paths
		// First, check if the candidate path improve Pareto-frontier for any time budget, if it is the case, it can be considered as local-reliable
		for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
			if( lt( _frontier.getCdfT(t) , candidatepath.getDistribT(t)[2] ) ){
				localreliable = true;
			}
		}
		// There, if the candidate path is not local-reliable, the analysis must be ended, and the functions must return 'false' value
		if( !localreliable ){
			//			TRACE("Candidate path is not local-reliable!");
			return localreliable;
		}
		// Here we know that the path is local-reliable, we still have to modify the routing policy so as to take into account the new path
		std::map<uint32_t,Path> pathToCounterCheck;
		// Compare the cdf values between candidate path and current best path, for each time budget
		for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
			TRACE("t = " << t*_frontier.getDelta() << ": " << _frontier.getCdfT(t) << " vs. " << candidatepath.getDistribT(t)[2] << "\t");
			uint32_t lastbestpathid( _bestpaths[t] );
			// If the candidate is strictly larger than the current best path at the t index
			if( le( _frontier.getCdfT(t) , candidatepath.getDistribT(t)[2] ) ){
				CONTINUE_STATUS("Decrement DSD of path P" << _bestpaths[t] << " at index " << t << " ; increment those of P" << candidateid << "\n");
				// Set the frontier cdf
				_frontier.setCdfT( t , candidatepath.getDistribT(t)[2] );
				// Decrement degree of strong dominance for defeated path, and increment those of candidate path
				_paths[lastbestpathid].decrementDSD();
				candidatepath.incrementDSD();
				// Set the best path for segment t as the candidate path id
				_bestpaths[t] = candidateid;
				pathToCounterCheck[lastbestpathid] = _paths[lastbestpathid] ;
			}
			// Do not forget that pdf must be updated accordingly!
			if(t == 0){
				if(!(le(0,_frontier.getCdfT(t))&&le(_frontier.getCdfT(t),1))){
					ERROR("Invalid pdf value at index t= " << t << "... " << _frontier.getCdfT(t) << " (old frontier pdf: " << _frontier.getPdfT(t) << ")\n");
				}
				_frontier.setPdfT( t , _frontier.getCdfT(t) );
			}else{
				if(!(le(0,_frontier.getCdfT(t)-_frontier.getCdfT(t-1))&&le(_frontier.getCdfT(t)-_frontier.getCdfT(t-1),1))){
					ERROR("Invalid pdf value at index t= " << t << "... " << _frontier.getCdfT(t)-_frontier.getCdfT(t-1) << " (old frontier pdf: " << _frontier.getPdfT(t) << ")\n");
				}
				_frontier.setPdfT( t , _frontier.getCdfT(t)-_frontier.getCdfT(t-1) );
			}
		}
		//		TRACE("Initial Cdf comparison: OK");
		// Counter-checking (phase 1): if path to check have a DSD equal to 0, erase them directly from the LR-path list (a,d from the path-to-counter-check list as well)
		for(auto itpcc = pathToCounterCheck.begin() ; itpcc != pathToCounterCheck.end() ; ){
			//			TRACE("Path to counter-check (P" << itpcc->first << "): " << itpcc->second);
			if( itpcc->second.getDSD() == 0 ){
				_paths.erase(itpcc->first);
				pathToCounterCheck.erase(itpcc++);
			}
			else{
				++itpcc;
			}
		}
		//		TRACE("Counter-checking phase 1: OK");
		// Counter-checking (phase 2): if there is still (a) path(s) to counter-check, look for substitute for each time budget such that they are still optimal
		if( pathToCounterCheck.size()>0 ){
			for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
				uint32_t lastbestpathid( _bestpaths[t]);
				// If a path to counter-check is the best solution for the current budget, try to find a substitute amongst other LR-path
				if( pathToCounterCheck.find(lastbestpathid) != pathToCounterCheck.end() ){
					for(auto& path: _paths){
						// If alternative path must be counter-checked too, do not evaluate it, go to the next one
						if( pathToCounterCheck.find(path.first) != pathToCounterCheck.end() ){
							continue;
						}
						// If there is another regular path that is optimal for this budget, set it as the new best path for this time period
						if( eq( _frontier.getCdfT(t) , path.second.getDistribT(t)[2] ) ){
							// Set the best path for segment t as the candidate path id
							_bestpaths[t] = path.first;
							// Decrement degree of strong dominance for defeated path, and increment those of candidate path
							_paths[lastbestpathid].decrementDSD();
							_paths[path.first].incrementDSD();
							break; // An alternative path has been found, go to the following budget
						}
					}
				}
				// If the current path to counter-check has now a null DSD, erase it from the LR-path list (and from the path-to-counter-check list)
				if( _paths[lastbestpathid].getDSD() == 0 ){
					_paths.erase(lastbestpathid);
					pathToCounterCheck.erase(lastbestpathid);
					// If the path-to-counter-check list is empty, end up the counter-check process
					if( pathToCounterCheck.size() == 0){
						break;
					}
				}
			}
		}
		//		TRACE("Counter-checking phase 2: OK");
		// If candidate path has a strictly positive degree of strong dominance, it must be added to the path container (at this point, it should be true)
		if( candidatepath.getDSD() > 0 ){
			_paths[ candidateid ] = candidatepath;
		}
		return localreliable;
	}
}

/*
 * addPaths(const RoutingPolicy&, const Edge_id&, const Distribution&) method: try to add new paths to the path list, return true if at least one path have been added
 * Starting from origin node routing policy, append current node to create new paths and compare them with existing paths of the policy
 */
bool RoutingPolicy::addPaths(const RoutingPolicy& rp, const Edge_id& e, const Distribution& dist){
	//	TRACE("[BEFORE] Current policy: " << *this);
	bool response( false );
	uint32_t candidateCounter( _paths.rbegin()->first + 1 );
	// For each candidate path (i.e. each path created by appending origin node routing policy paths with current node n)
	for(auto& path: rp.getPaths()){
		// Create the candidate path, and set its id according to the amount of existing path in the current node routing policy
		Path candidatepath = Path(path.second,_node,e,dist);
		candidatepath.setDSD(0);
		uint32_t candidateid = ++candidateCounter;
		//		TRACE("Candidate path (id=" << candidateid << "): " << candidatepath);
		// Try to add the path to the current routing policy, set the response as true if the candidate path is local reliable
		bool pathadded( addPath(candidatepath, candidateid) );
		response = response || pathadded;
		//		TRACE("[PATH " << (pathadded?"":"NOT ") << "ADDED] Current policy: " << *this);
		//		response |= addPath(candidatepath, candidateid);
	}
	//	TRACE("[AFTER] Current policy: " << *this);
	return response;
}

/*
 * append() method: append the current routing policy with the one given as a parameter
 * This procedure aims at appending a forward routing policy (between start node and a node in the top of the hierarchy)
 * with a backward routing policy (between destination and the same top-level node)
 */
RoutingPolicy RoutingPolicy::append(const RoutingPolicy& bwrp){
	assert( _node == bwrp.getNode() );
	std::map<uint32_t,Path> bwpaths =  bwrp.getPaths();
	Node_id destination = bwpaths.begin()->second.getFirstNode();
	RoutingPolicy fullroutepolicy(destination , _frontier.getSize() , _frontier.getDelta());
	uint32_t pathcounter(0);
	for(auto& fwpath: _paths){
		for(auto& bwpath: bwpaths){
			Path newCompletePath = fwpath.second.append(bwpath.second);
//			TRACE("New complete path: " << newCompletePath);
			// If path is acyclic, include it within the policy
			if( newCompletePath.acyclic() ){
				fullroutepolicy.addPath(newCompletePath, ++pathcounter);
			}
//			TRACE( fullroutepolicy );
		}
	}
	return fullroutepolicy;
}

/*
 * merge(const RoutingPolicy&) method: merge the current routing policy with the candidate given as a parameter
 * Concretely, add the candidate policy paths to the vector of current paths, and modify accordingly the '_frontier' and '_bestpaths' structures
 */
void RoutingPolicy::merge(const RoutingPolicy& candidatePolicy){
	assert( _node == candidatePolicy.getNode());
	if( _paths.size() == 0 ){
//		TRACE("Try to merge an empty policy...");
		_node = candidatePolicy.getNode();
		_paths = candidatePolicy.getPaths();
		_frontier = candidatePolicy.getFrontier();
		_bestpaths = candidatePolicy.getBestPathIds();
	}
	else{
		for(auto& candidatepath: candidatePolicy.getPaths() ){
			candidatepath.second.setDSD(0);
			uint32_t candidatepathid( _paths.rbegin()->first + 1 );
			addPath(candidatepath.second, candidatepathid);
		}
	}
}

/*
 * removeUselessPaths(std::set<uint32_t>&, const std::set<uint32_t>& ) method: remove paths identified by the first index set given as the first parameter
 * the second parameter refers to IDs of valid paths; these paths are potential substitutes of removed paths
 */
void RoutingPolicy::removeUselessPaths(std::set<uint32_t>& uselessPathIDs, const std::set<uint32_t>& usefulPathIDs){
	//	TRACEF("Useful path IDs: ");
	//	for(auto& pathID: usefulPathIDs){
	//		CONTINUE_STATUS(" " << pathID);
	//	}
	//	CONTINUE_STATUS("\n");
	//	TRACEF("Useful path IDs: ");
	//	for(auto& pathID: uselessPathIDs){
	//		CONTINUE_STATUS(" " << pathID);
	//	}
	//	CONTINUE_STATUS("\n");
	for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
		uint32_t lastbestpathid( _bestpaths[t]);
		// If a path to counter-check is the best solution for the current budget, try to find a substitute amongst other LR-path
		if( uselessPathIDs.find(lastbestpathid) != uselessPathIDs.end() ){
			//			TRACE("An useless path is marked as LR at index t = " << t << "(best path: P" << lastbestpathid << ")" );
			for(auto& pathID: usefulPathIDs){
				// If there is another regular path that is optimal for this budget, set it as the new best path for this time period
				if( eq( _frontier.getCdfT(t) , _paths[pathID].getDistribT(t)[2] ) ){
					// Set the best path for segment t as the candidate path id
					_bestpaths[t] = pathID;
					// Decrement degree of strong dominance for defeated path, and increment those of substitute path
					_paths[lastbestpathid].decrementDSD();
					_paths[pathID].incrementDSD();
					//					CONTINUE_STATUS("Decrement DSD of path P" << lastbestpathid << " at index " << t << " ; increment those of P" << pathID << "\n");
					break; // An alternative path has been found, go to the following budget
				}
			}
			// If the current path to counter-check has now a null DSD, erase it from the LR-path list (and from the path-to-counter-check list)
			if( _paths[lastbestpathid].getDSD() == 0 ){
				_paths.erase( lastbestpathid );
				uselessPathIDs.erase( lastbestpathid );
				// If the path-to-counter-check list is empty, end up the counter-check process
				if( uselessPathIDs.size() == 0){
					break;
				}
			}
		}
		//		TRACE("At index t=" << t << ", we have " << uselessPathIDs.size() << " useless path(s) and " << usefulPathIDs.size() << " useful paths.");
	}
}

/*
 * clean() method: erase path that are considered as local-reliable only for time budgets such that other optimal paths give comparable probability to be on-time
 * Final check: some paths may still be in the list by error (just because of equal cdf values, whereas dominance check strict inequalities => e.g. case when cdf=0 or =1)
 */
void RoutingPolicy::clean(const Specif& specif){
	// Initialize cleaning process: all paths in the policy are considered as 'useless'
	//	std::map<uint32_t,Path> usefulPaths;
	std::set<uint32_t> usefulPathIDs;
	std::set<uint32_t> uselessPathIDs;
	for(auto& path: _paths){
		uselessPathIDs.insert( path.first );
	}
	// For each time budget, check if the optimal path is the only one (if it is the case, consider that the path is not useless)
	for( uint32_t timebudget(0) ; timebudget < _frontier.getSize() ; ++timebudget ){
		//		TRACE("Pareto frontier for t = " << timebudget << ": " << _frontier.getCdfT(timebudget) );
		uint32_t optPathCount(0);
		// Amongst all paths, count the paths that contribute to the current frontier cdf value
		for(auto& path: _paths){
			//			TRACE( "Probability to be on-time at t = " << timebudget << " for path P" << path.first << ": " << path.second.getDistribT(timebudget)[2] );
			if( eq( path.second.getDistribT(timebudget)[2] , _frontier.getCdfT(timebudget) ) ){
				//				TRACE("Path P" << path.first << " contributes to Pareto frontier at index t = " << timebudget);
				++optPathCount;
			}
			if( optPathCount>1 ){
				break;
			}
		}
		// There must be at least one path for each index, if there is only one path, that means that the path is useful
		assert(optPathCount >= 1);
		if( optPathCount == 1){
			usefulPathIDs.insert( _bestpaths[timebudget] );
			uselessPathIDs.erase( _bestpaths[timebudget] );
		}
		// If all paths are marked as 'useful', end up the cleaning process by keeping every path of the policy
		if( usefulPathIDs.size() == _paths.size() ){
			assert( uselessPathIDs.size() == 0);
			return;
		}
		// If the frontier cdf reaches 1, change are not expected any more
		if( eq( _frontier.getCdfT(timebudget) , 1) ){
			break;
		}
	}
	// Here we know which paths are optimal, and which one can be deleted from the optimal policy, call the 'removePath' procedure on each of useless paths
	removeUselessPaths( uselessPathIDs , usefulPathIDs );
}

/*
 * developPaths(const Graph*) method: develop each local-reliable paths of the current routing policy
 */
void RoutingPolicy::developPaths(const Graph* g){
	//	STATUS("Develop paths in the final routing policy...\n");
	for(auto& path: _paths){
		path.second.develop( g );
	}
}

/*
 * serialize(std::string& , std::string&) method: save the routing policy in two text files (one for routing policy distribution and one for path descriptions)
 * format routing policy: [time budget t] <frontier.cdf[t]> <bestpathid[t]> <path1.cdf[t]> <path2.cdf[t]> ...
 * format path nodes: [path p] <p.id> <p.node[1]> <p.node[2]> ... <p.node[N]> <NA> ... <NA>
 * In this second file, complete the lines with as NA value as necessary (a line must contain as many node indices as the longer path
 */
void RoutingPolicy::serialize(std::string& rpfilename, std::string& pnfilename){
	std::ofstream outputfile1(rpfilename);
	std::ofstream outputfile2(pnfilename);
	if ( ! outputfile1.is_open() ){
		ERROR("Unable to open file RP.\n");
	}
	if ( ! outputfile2.is_open() ){
		ERROR("Unable to open file PN.\n");
	}
	uint32_t maxpathlength(0);
	// Header of RP file
	outputfile1 << "Frontier" << " " << "Best" << " ";
	for(auto path: _paths){
		maxpathlength = std::max( maxpathlength, path.second.getLength() );
		outputfile1 << "P" << path.first << " ";
		outputfile2 << "P" << path.first << " ";
	}
	outputfile1 << "\n";
	outputfile2 << "\n";
	// Routing policy description (only cdf is kept, pdf can be deduced from it)
	for(uint32_t t(0) ; t < _frontier.getSize() ; ++t){
		outputfile1 << _frontier.getCdfT(t) << " P" << _bestpaths[t] << " ";
		for(auto path: _paths){
			outputfile1 << path.second.getDistribT(t)[2] << " ";
		}
		outputfile1 << "\n";
	}
	// Path nodes description
	for(uint32_t i( 0 ) ; i < maxpathlength ; ++i){
		for(auto path: _paths){
			std::stringstream curnodeid;
			if( i < path.second.getLength() ){
				outputfile2 << path.second.getNode(i) << " ";
			}
			else{
				outputfile2 << "NA ";
			}
		}
		outputfile2 << "\n";
	}
	// Finalization: file closing
	outputfile1.close();
	outputfile2.close();
}

/*
 * << operator: return an outstream version of the routing policy (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const RoutingPolicy& rp){
	os << "*** Routing policy for node N" << rp._node << "\n";
	for(auto& path: rp._paths){
		os << "Path P" << path.first << ": " << path.second << "\n";
	}
	os << "Frontier: " << rp._frontier << "\n";
	os << "Best paths: ";
	for(auto& id: rp._bestpaths){
		os << "P" << id << " ";
	}
	os << "\n";
	return os;
}
