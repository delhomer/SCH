/*
 * spotarpolicy.cpp
 *
 *  Created on: 29 mars 2016
 *      Author: delhome
 */

#include "spotarpolicy.h"

#include <set>

SpotarPolicy::SpotarPolicy(){}
SpotarPolicy::SpotarPolicy(const Node_id& n, const int& size, const uint32_t& delta, const bool& empty): _node(n){
	if(empty){
		_frontier = Distribution(size,delta);
		_bestpaths.assign(size,std::numeric_limits<uint32_t>::max());
	}
	else{
		_paths[1] = SpotarPath(1,n,size,delta);
		_LRpaths[1] = SpotarPath(1,n,size,delta);
		_frontier = _paths[1].getDistribution();
		_bestpaths.assign( _frontier.getSize() , 1 );
	}
}
SpotarPolicy::SpotarPolicy(const SpotarPath& path): _node(path.getFirstNode()){
	_paths[ path.getId() ] = path;
	_LRpaths[ path.getId() ] = path;
	_frontier = path.getDistribution();
	_bestpaths.assign( _frontier.getSize() , path.getId() );
}

/*
 * Getters
 */
Node_id SpotarPolicy::getNode() const{ return _node; }
Distribution SpotarPolicy::getFrontier() const{ return _frontier; }
double SpotarPolicy::getShortestTime() const { return _frontier.min(); }
double SpotarPolicy::getLargestTime() const { return _frontier.max(); }
std::array<double,3> SpotarPolicy::getFrontierT(const uint32_t& index) const{
	std::array<double,3> frontierElement;
	frontierElement[0] = _frontier.getSupT(index);
	frontierElement[1] = _frontier.getPdfT(index);
	frontierElement[2] = _frontier.getCdfT(index);
	return frontierElement;
}
SpotarPath SpotarPolicy::getFirstPath() const { return _LRpaths.begin()->second; }
std::map<uint32_t,SpotarPath> SpotarPolicy::getPaths() const { return _paths; }
std::map<uint32_t,SpotarPath> SpotarPolicy::getLRPaths() const { return _LRpaths; }
std::vector<uint32_t> SpotarPolicy::getBestPathIds() const { return _bestpaths; }
uint32_t SpotarPolicy::getBestPathId(const uint32_t& index) const { return _bestpaths[index]; }
uint32_t SpotarPolicy::getNbPaths() const{ return _paths.size(); }
uint32_t SpotarPolicy::getNbLRPaths() const{ return _LRpaths.size(); }

/*
 * Setters
 */
void SpotarPolicy::setNode(const Node_id& n){ _node = n; }
void SpotarPolicy::setFrontier(const Distribution& dist){ _frontier = dist; }
void SpotarPolicy::setFrontierT(const uint32_t& index, const double& pdfValue, const double& cdfValue){
	_frontier.setPdfT(index,pdfValue);
	_frontier.setCdfT(index,cdfValue);
}
void SpotarPolicy::getBestPathIds(const std::vector<uint32_t>& bp){
	_bestpaths = bp;
}

/*
 * empty() method: return true if there is not any candidate path to check in the policy, false otherwise
 */
bool SpotarPolicy::empty(){
	return _LRpaths.size() == 0;
}

/*
 * clear() method: delete the saved paths and reset the frontier as an infinite distribution
 */
void SpotarPolicy::clear(){
	_LRpaths.clear();
	_paths.clear();
	_frontier.makeInfinite();
}

/*
 * addPath(const SpotarPath&) method: add a new path to the routing policy
 */
void SpotarPolicy::addPath(const SpotarPath& path){
	_paths[path.getId()] = path;
}

/*
 * addlrpath(SpotarPath&) method: add the path in the candidate set if it is local-reliable and update this set accordingly by erasing obsolete paths
 * return true if the path is really local-reliable, false otherwise
 */
bool SpotarPolicy::addLRpath(SpotarPath& candidatepath){
//	TRACE("Check if following path is local-reliable: " << candidatepath);
	// Initialization
	bool localreliable(false);
	uint32_t firstLRindex(-1);
	for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
		uint32_t lastbestpathid( _bestpaths[t] );
		// If the candidate is strictly larger than the current best path at the t index
		if( lt( _frontier.getCdfT(t) , candidatepath.getDistribT(t)[2] ) ){
			if(!localreliable){
				firstLRindex = t;
			}
			localreliable = true;
			// TRACE("Decrement DSD of path P" << _bestpaths[t] << " at index " << t << " ; increment those of P" << candidatepath.getId() );
			// Set the frontier cdf
			_frontier.setCdfT( t , candidatepath.getDistribT(t)[2] );
			// Set the best path for segment t as the candidate path id
			_bestpaths[t] = candidatepath.getId();
			// Decrement degree of strong dominance for defeated path, and increment those of candidate path
			_paths[lastbestpathid].decrementDSD();
			_LRpaths[lastbestpathid].decrementDSD();
			candidatepath.incrementDSD();
			// If the defeated path is not the best one for any segment, erase it from the path container
			if( _paths[lastbestpathid].getDSD() == 0 ){
				_LRpaths.erase( lastbestpathid );
			}
		}
		else if( localreliable == true && eq( _frontier.getCdfT(t) , candidatepath.getDistribT(t)[2] ) ){
			// TRACE("Decrement DSD of path P" << _bestpaths[t] << " at index " << t << " ; increment those of P" << candidatepath.getId() );
			// Set the frontier cdf
			_frontier.setCdfT( t , candidatepath.getDistribT(t)[2] );
			// Set the best path for segment t as the candidate path id
			_bestpaths[t] = candidatepath.getId();
			// Decrement degree of strong dominance for defeated path, and increment those of candidate path
			_paths[lastbestpathid].decrementDSD();
			_LRpaths[lastbestpathid].decrementDSD();
			candidatepath.incrementDSD();
			// If the defeated path is not the best one for any segment, erase it from the path container
			if( _paths[lastbestpathid].getDSD() == 0 ){
				_LRpaths.erase( lastbestpathid );
			}
		}
		// Do not forget that pdf must be updated accordingly!
		if(t == 0){
			_frontier.setPdfT( t , _frontier.getCdfT(t) );
		}else{
			_frontier.setPdfT( t , _frontier.getCdfT(t)-_frontier.getCdfT(t-1) );
		}
	}
	if(localreliable){
		assert( firstLRindex >= 0 && firstLRindex < _frontier.getSize() );
		for( uint32_t t(0) ; t < firstLRindex ; ++t ){
			uint32_t lastbestpathid( _bestpaths[t] );
			if( eq( _frontier.getCdfT(t) , candidatepath.getDistribT(t)[2] ) ){
				// TRACE("Decrement DSD of path P" << _bestpaths[t] << " at index " << t << " ; increment those of P" << candidatepath.getId() );
				// Set the frontier cdf
				_frontier.setCdfT( t , candidatepath.getDistribT(t)[2] );
				// Set the best path for segment t as the candidate path id
				_bestpaths[t] = candidatepath.getId();
				// Decrement degree of strong dominance for defeated path, and increment those of candidate path
				_paths[lastbestpathid].decrementDSD();
				_LRpaths[lastbestpathid].decrementDSD();
				candidatepath.incrementDSD();
				// If the defeated path is not the best one for any segment, erase it from the path container
				if( _paths[lastbestpathid].getDSD() == 0 ){
					_LRpaths.erase( lastbestpathid );
				}
			}
			// Do not forget that pdf must be updated accordingly!
			if(t == 0){
				_frontier.setPdfT( t , _frontier.getCdfT(t) );
			}else{
				_frontier.setPdfT( t , _frontier.getCdfT(t)-_frontier.getCdfT(t-1) );
			}
		}
	}

	// If the path is local-reliable, add it into the LR-path list
//	TRACE("Candidate path " << (localreliable?"is":"is not") << " local-reliable!");
	if( localreliable ){
		_LRpaths[ candidatepath.getId() ] = candidatepath;
	}
	return localreliable;
}

/*
 * lrcheck(const SpotarPath&) method: add a candidate path in the candidate set
 */
bool SpotarPolicy::lrcheck(SpotarPath& candidatepath){
//	TRACE("Check if following path is local-reliable: " << candidatepath);
	// Initialization
	bool localreliable(false);
	// Update Pareto-frontier and identify paths that have to be erased
	for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
		uint32_t lastbestpathid( _bestpaths[t] );
		// If the candidate is strictly larger than the current best path at the t index
		if( lt( _frontier.getCdfT(t) , candidatepath.getDistribT(t)[2] ) ){
			localreliable = true;
			//			TRACE("Decrement DSD of path P" << _bestpaths[t] << " at index " << t << " ; increment those of P" << candidatepath.getId() );
			// Set the frontier cdf
			_frontier.setCdfT( t , candidatepath.getDistribT(t)[2] );
			// Set the best path for segment t as the candidate path id
			_bestpaths[t] = candidatepath.getId();
			// Decrement degree of strong dominance for defeated path, and increment those of candidate path
			_paths[lastbestpathid].decrementDSD();
			_LRpaths[lastbestpathid].decrementDSD();
			candidatepath.incrementDSD();
			// If the defeated path is not the best one for any segment, erase it from the path container
			if( _paths[lastbestpathid].getDSD() == 0 ){
				//				TRACE("Path P" << lastbestpathid << " has to be erased!");
				_LRpaths.erase( lastbestpathid );
			}
		}
		// Do not forget that pdf must be updated accordingly!
		if(t == 0){
			_frontier.setPdfT( t , _frontier.getCdfT(t) );
		}else{
			_frontier.setPdfT( t , _frontier.getCdfT(t)-_frontier.getCdfT(t-1) );
		}
	}
//	TRACE("Candidate path " << (localreliable?"is":"is not") << " local-reliable!");
	if( localreliable ){
		_LRpaths[ candidatepath.getId() ] = candidatepath;
	}
	return localreliable;
}

/*
 * lrcheck_NieWu(const SpotarPath&) method: add a candidate path in the candidate set
 */
bool SpotarPolicy::lrcheck_NieWu(SpotarPath& candidatepath){
//	TRACE("Check if following path is local-reliable (method implemented in (Nie,Wu;2009)): " << candidatepath);
	// Initialization
	bool localreliable(false);
	std::set<SpotarPath> erasedpaths;
	// Update Pareto-frontier and identify paths that have to be erased
	for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
		uint32_t lastbestpathid( _bestpaths[t] );
		// If the candidate is strictly larger than the current best path at the t index
		if( lt( _frontier.getCdfT(t) , candidatepath.getDistribT(t)[2] ) ){
			localreliable = true;
//			TRACE("Decrement DSD of path P" << _bestpaths[t] << " at index " << t << " ; increment those of P" << candidatepath.getId() );
			// Set the frontier cdf
			_frontier.setCdfT( t , candidatepath.getDistribT(t)[2] );
			// Set the best path for segment t as the candidate path id
			_bestpaths[t] = candidatepath.getId();
			// Decrement degree of strong dominance for defeated path, and increment those of candidate path
			_paths[lastbestpathid].decrementDSD();
			_LRpaths[lastbestpathid].decrementDSD();
			candidatepath.incrementDSD();
			// If the defeated path is not the best one for any segment, erase it from the path container
			if( _paths[lastbestpathid].getDSD() == 0 ){
//				TRACE("Add path P" << lastbestpathid << " to the set of paths to be erased...");
				erasedpaths.insert( _LRpaths[lastbestpathid] );
			}
		}
		// Do not forget that pdf must be updated accordingly!
		if(t == 0){
			_frontier.setPdfT( t , _frontier.getCdfT(t) );
		}else{
			_frontier.setPdfT( t , _frontier.getCdfT(t)-_frontier.getCdfT(t-1) );
		}
	}
	// Update the set of LR paths
	auto itep = erasedpaths.begin();
	while( localreliable && itep != erasedpaths.end()){//!erasedpaths.empty() ){
		// Consider the first path to erase
		//		SpotarPath curpath = erasedpaths.front();
		//		erasedpaths.pop();
		SpotarPath curpath = *itep;
//		TRACE("LR-analysis on path " << curpath);
		// Initialize comparison counters
		uint32_t n_l(0), n_e(0), n_g(0);
		// For each time budget, compare current path to erase and candidate path
		for( uint32_t t(0) ; t < _frontier.getSize() ; ++t ){
			// Continue the comparison while inferiority and superiority counters are equal to 0
			if( n_l != 0 && n_g != 0 ){
				break;
			}
			// Case 1: candidate path is better than current path for this time budget => increment superiority counter
			if( lt( curpath.getDistribT(t)[2] , candidatepath.getDistribT(t)[2] ) ){
//				TRACE("Index t=" << t << ": Increment SUP!");
				++n_g;
			}
			// Case 2: candidate path and current path are equal => increment equality counter
			else if( eq( curpath.getDistribT(t)[2] , candidatepath.getDistribT(t)[2] ) ){
				++n_e;
			}
			// Case 3: candidate path is worst than current path for this time budget => increment inferiority counter
			else{
//				TRACE("Index t=" << t << ": Increment INF!");
				++n_l;
			}
		}
		// If superiority counter is equal to 0, candidate path is never better than current LR-path, local reliable is set to false
		if( n_l == 0){
			localreliable = false;
		}
		// If inferiority counter is equal to 0, candidate path is never worst than current path, the latter can be erased from LR-path set
		else if(n_g == 0){
			_LRpaths.erase( curpath.getId() );
		}
		++itep;

	}
	if( localreliable ){
		_LRpaths[ candidatepath.getId() ] = candidatepath;
	}
//	TRACE("Candidate path " << (localreliable?"is":"is not") << " local-reliable!");
	return localreliable;
}

/*
 * removeUselessPaths(std::set<uint32_t>&, const std::set<uint32_t>& ) method: remove paths identified by the first index set given as the first parameter
 * the second parameter refers to IDs of valid paths; these paths are potential substitutes of removed paths
 */
void SpotarPolicy::removeUselessPaths(std::set<uint32_t>& uselessPathIDs, const std::set<uint32_t>& usefulPathIDs){
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
				if( eq( _frontier.getCdfT(t) , _LRpaths[pathID].getDistribT(t)[2] ) ){
					// Set the best path for segment t as the candidate path id
					_bestpaths[t] = pathID;
					// Decrement degree of strong dominance for defeated path, and increment those of substitute path
					_LRpaths[lastbestpathid].decrementDSD();
					_LRpaths[pathID].incrementDSD();
//					CONTINUE_STATUS("Decrement DSD of path P" << lastbestpathid << " at index " << t << " ; increment those of P" << pathID << "\n");
					break; // An alternative path has been found, go to the following budget
				}
			}
			// If the current path to counter-check has now a null DSD, erase it from the LR-path list (and from the path-to-counter-check list)
			if( _LRpaths[lastbestpathid].getDSD() == 0 ){
				_LRpaths.erase( lastbestpathid );
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
void SpotarPolicy::clean(const Specif& specif){
	// Initialize cleaning process: all paths in the policy are considered as 'useless'
//	std::map<uint32_t,Path> usefulPaths;
	std::set<uint32_t> usefulPathIDs;
	std::set<uint32_t> uselessPathIDs;
	for(auto& path: _LRpaths){
		uselessPathIDs.insert( path.first );
	}
	// For each time budget, check if the optimal path is the only one (if it is the case, consider that the path is not useless)
	for( uint32_t timebudget(0) ; timebudget < _frontier.getSize() ; ++timebudget ){
//		TRACE("Pareto frontier for t = " << timebudget << ": " << _frontier.getCdfT(timebudget) );
		uint32_t optPathCount(0);
		// Amongst all paths, count the paths that contribute to the current frontier cdf value
		for(auto& path: _LRpaths){
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
		if( usefulPathIDs.size() == _LRpaths.size() ){
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
 * << operator: return an outstream version of the routing policy (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const SpotarPolicy& sp){
	os << "*** Routing policy for node N" << sp._node << "\n";
	for(auto& path: sp._paths){
		os << (sp._LRpaths.find(path.first)!=sp._LRpaths.end()?"[LR-PATH] ":"") << path.second << "\n";
	}
	os << "Frontier: " << sp._frontier << "\n";
	os << "Best paths: ";
	for(auto& id: sp._bestpaths){
		os << "P" << id << " ";
	}
	os << "\n";
	return os;
}
