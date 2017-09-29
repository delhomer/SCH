/*
 * misc.h
 *
 *  Created on: 2 févr. 2016
 *      Author: delhome
 */

#ifndef MISC_H_
#define MISC_H_

#include <assert.h>
#include <algorithm>
#include <boost/serialization/strong_typedef.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <math.h>


/*
 * Miscellaneous file
 * Contains generic functions and declarations useful in the whole project
 */

/*
 * Trace definition
 */
#define VERBOSITY1 true
#define VERBOSITY2 true
#define VERBOSITY3 true
#define MARK(s) if (VERBOSITY1) std::cout << "***** " << s << " *****\n" << std::flush
#define STATUS(s) if (VERBOSITY2) std::cout << "%%%%%%% STATUS [" << __FILE__ << ":" << __LINE__  << "] " << s << std::flush
#define WARNING(s) if (VERBOSITY2) std::cerr << "WARNING [" << __FILE__ << ":" << __LINE__  << "] " << s << std::flush
#define ERROR(s) if (VERBOSITY2) std::cerr << "ERROR [" << __FILE__ << ":" << __LINE__  << "] " << s << std::flush
#define CONTINUE_STATUS(s) if (VERBOSITY2) std::cout << s << std::flush
#define CONTINUE_ERROR(s) if (VERBOSITY2) std::cerr << s << "\n" << std::flush
#define CONTINUE_WARNING(s) if (VERBOSITY2) std::cerr << s << "\n" << std::flush
#define TRACEF(s) if (VERBOSITY3) std::cout << "===> " << s << std::flush
#define TRACE(s) if (VERBOSITY3) std::cout << "===> " << s << "\n" << std::flush

/*
 * Double comparisons
 */
static constexpr double EPSILON = 0.000001;

inline bool le(const double& x, const double& y) { return x - y <= EPSILON; }
inline bool lt(const double& x, const double& y) { return x - y < -EPSILON; }
inline bool eq(const double& x, const double& y) { return fabs(x - y) <= EPSILON; }
inline bool neq(const double& x, const double& y) { return ! eq(x, y); }
inline bool gt(const double& x, const double& y) { return lt(y, x); }
inline bool ge(const double& x, const double& y) { return le(y, x); }

/*
 * remove_duplicates(vector<T>&, const LtFn&, cont EqFn&) method: remove duplicate values in a vector where elements are comparable ("less than" and "equal" comparisons)
 */
template <typename T, typename LtFn = std::less<T>, typename EqFn = std::equal_to<T>>
void remove_duplicates(std::vector<T>& vec, const LtFn& lt = std::less<T>(), const EqFn& eq = std::equal_to<T>()){
    std::sort(vec.begin(), vec.end(), lt);
    auto new_end = std::unique(vec.begin(), vec.end(), eq);
    vec.resize( std::distance(vec.begin(), new_end) );
}

/*
 * Definition of two specific types: Node_id and Edge_id, defined as double
 */
BOOST_STRONG_TYPEDEF(uint32_t, Node_id);
const Node_id INVALID_NODE_ID(std::numeric_limits<uint32_t>::max());
BOOST_STRONG_TYPEDEF(uint32_t, Edge_id);
const Edge_id INVALID_EDGE_ID( std::numeric_limits<uint32_t>::max() );

/*
 * Class Filler: functor useful to fill a container with incremental indices
 */
class Filler{
private:
	size_t coefficient;
public:
	Filler(int starter):coefficient(starter){}
	void reinitialize(){
		coefficient = 0;
	}
	int operator()() {
		return coefficient++;
	}
};

/*
 * Chrono
 */
inline std::chrono::high_resolution_clock::time_point time_stamp(){
    return std::chrono::high_resolution_clock::now();
}

inline double get_duration_in_milliseconds(const std::chrono::high_resolution_clock::time_point& from, const std::chrono::high_resolution_clock::time_point& to){
    assert( from <= to );
    return std::chrono::duration_cast<std::chrono::milliseconds>(to - from).count();
}

inline double get_duration_in_seconds(const std::chrono::high_resolution_clock::time_point& from, const std::chrono::high_resolution_clock::time_point& to){
    assert( from <= to );
    return std::chrono::duration_cast<std::chrono::duration<double>>(to - from).count();
}


#endif /* MISC_H_ */
