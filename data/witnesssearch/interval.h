/*
 * interval.h
 *
 *  Created on: 10 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_INTERVAL_H_
#define DATA_INTERVAL_H_

#include <algorithm> // Command min, max
#include <limits>       // std::numeric_limits
#include <ostream>

class Interval{

public:
	/*
	 * Constructors
	 */
    Interval();
    Interval(const double lower, const double upper);

    /*
     * Getters
     */
    double getLB() const;
    double getUB() const;

    /*
     * Setter
     */
    void setInterval(const double& lb, const double& ub);

    /*
     * isInfinite() method: return true if the current interval has infinite bounds
     */
    bool isInfinite();

    /*
     * merge(const Interval&, const Interval&) method: return an interval composed by the minimum of provided interval lower and upper bounds
     */
    friend Interval merge(const Interval& lhs, const Interval& rhs);

    /*
     * merge(const Interval&, const Interval&) method: return an interval composed by the minimum of provided interval lower and upper bounds
     */
    friend Interval intersect(const Interval& lhs, const Interval& rhs);

    /*
     * merge(const Interval&, const Interval&) method: return an interval composed by the minimum of provided interval lower and upper bounds
     */
    friend Interval unite(const Interval& lhs, const Interval& rhs);

    /*
     * == operator: comparison purpose, return true if bounds are equal in both provided interval
     */
    friend bool operator== (const Interval& lhs, const Interval& rhs);

    /*
     * != operator: comparison purpose
     */
    friend bool operator!= (const Interval& lhs, const Interval& rhs);

    /*
     * << operator: return an outstream version of the interval (printing purpose)
     */
    friend std::ostream& operator<<(std::ostream& os, const Interval& interval);

private:
    /*
     * Attributes
     */
    double _lower;
    double _upper;

};


#endif /* DATA_INTERVAL_H_ */
