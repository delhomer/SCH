/*
 * interval.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "interval.h"

/*
 * Constructors
 */
Interval::Interval(): _lower(std::numeric_limits<double>::max()), _upper(std::numeric_limits<double>::max()){}
Interval::Interval(const double lower, const double upper): _lower(lower), _upper(upper){}

/*
 * Getters
 */
double Interval::getLB() const{ return _lower; }
double Interval::getUB() const{ return _upper; }

/*
 * Setter
 */
void Interval::setInterval(const double& lb, const double& ub){ _lower = lb; _upper = ub; }

/*
 * isInfinite() method: return true if the current interval has infinite bounds
 */
bool Interval::isInfinite(){
	return _lower == std::numeric_limits<double>::max() && _upper == std::numeric_limits<double>::max() ;
}

/*
 * merge(const Interval&, const Interval&) method: return an interval composed by the minimum of provided interval lower and upper bounds
 */
Interval merge(const Interval& lhs, const Interval& rhs){
	return Interval( std::min(lhs.getLB(), rhs.getLB()), std::min(lhs.getUB(), rhs.getUB()) );
}

/*
 * intersect(const Interval&, const Interval&) method: return an interval composed by the minimum of provided interval lower and upper bounds
 */
Interval intersect(const Interval& lhs, const Interval& rhs){
	return Interval( std::max(lhs.getLB(), rhs.getLB()), std::min(lhs.getUB(), rhs.getUB()) );
}

/*
 * unite(const Interval&, const Interval&) method: return an interval composed by the minimum of provided interval lower and upper bounds
 */
Interval unite(const Interval& lhs, const Interval& rhs){
	return Interval( std::min(lhs.getLB(), rhs.getLB()), std::max(lhs.getUB(), rhs.getUB()) );
}

/*
 * == operator: comparison purpose, return true if bounds are equal in both provided interval
 */
bool operator== (const Interval& lhs, const Interval& rhs){
	return lhs._lower == rhs._lower && lhs._upper == rhs._upper;
}

/*
 * != operator: comparison purpose
 */
bool operator!= (const Interval& lhs, const Interval& rhs){
	return ! ( lhs == rhs );
}

/*
 * << operator: return an outstream version of the interval (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const Interval& inter){
    os << "[" << inter._lower << "," << inter._upper << "]";
    return os;
}
