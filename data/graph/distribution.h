/*
 * cdf.h
 *
 *  Created on: 3 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_DISTRIBUTION_H_
#define DATA_DISTRIBUTION_H_

#include <algorithm> // Command transform, max
#include <cstdlib>
#include <functional>
#include <fstream>
#include <iostream>
#include <numeric>      // std::partial_sum, std::inner_product
#include <vector>

#include "../../misc.h"

/*
 * class Distribution: describe a cumulative distribution function, ie the probability of taking a value smaller than one x, x given
 * Let note F(.) such a function. By definition, lim[-inf]F(.)=0 ; lim[+inf]F(.)=1 ; F(.) is increasing.
 * Here the cdf are supposed as constant-by-step functions (as we consider only discretized variables).
 */
class Distribution{
public:
	/*
	 * Constructors
	 */
	Distribution();
	Distribution(const int& size);
	Distribution(const int& size, const int& delta);
	Distribution(const int& size, const int& delta, const bool& infinite);
	Distribution(const std::vector<uint32_t>& x, const std::vector<double>& p, const std::vector<double>& u);
	Distribution(const std::string& filename);

	/*
	 * Destructor
	 */
	~Distribution();

	/*
	 * Getters
	 */
	std::vector<uint32_t> getSup() const;
	std::vector<double> getCdf() const;
	std::vector<double> getPdf() const;
	uint32_t getSupT(const uint32_t& t) const;
	double getCdfT(const uint32_t& t) const;
	double getPdfT(const uint32_t& t) const;

	/*
	 * Setters
	 */
	void setSup(std::vector<uint32_t> newSup);
	void setPdf(std::vector<double> newPdf);
	void setCdf(std::vector<double> newCdf);
	void setSupT(uint32_t index, uint32_t newValue);
	void setPdfT(uint32_t index, double newValue);
	void setCdfT(uint32_t index, double newValue);

	/*
	 * isInfinite() method: check if the distribution is "infinite", that means the only possible value is the maximum of the admitted range [0;Tmax], ie cdf(t)=1 if t=Tmax, cdf(t)=0 otherwise
	 */
	bool isInfinite();

	/*
	 * reset() method: reset the distribution
	 */
	void reset();

	/*
	 * makeInfinite() method: set the distribution at the "infinite" status
	 * Here infinite means the same size, but the only support point with a non-nul probability is the maximal one
	 */
	void makeInfinite();

	/*
	 * getSize() method: return the number of support points
	 */
	uint32_t getSize() const;

	/*
	 * getDelta() method: return the difference between each support points (defined as a constant value)
	 */
	uint32_t getDelta() const;

	/*
	 * getSupI(uint32_t) method: return the i^th support point
	 */
	uint32_t getSupI(uint32_t index) const;

	/*
	 * getProbI(double) method: return the i^th step of the cdf
	 */
	double getPdfI(uint32_t index) const;

	/*
	 * getCumI(double) method: return the i^th step of the cdf
	 */
	double getCdfI(uint32_t index) const;

	/*
	 * Cdf(uint32_t) method: gives the probability of having a value less than the parameter
	 */
	double Cdf(uint32_t t) const;

	/*
	 * Pdf(uint32_t) method: gives the probability of having a value equal to parameter
	 */
	double Pdf(uint32_t t) const;

	/*
	 * min() method: return the minimum possible realization
	 */
	uint32_t min() const;

	/*
	 * max() method: return the maximum possible realization
	 */
	uint32_t max() const;

	/*
	 * range() method: return the definition range of the distribution, namely max()-min()
	 */
	uint32_t range() const;

	/*
	 * esp() method: return the esperance of the distribution
	 * E(T) = sum_t=0->Tmax_{t*p(t)}
	 */
	double esp() const;
	/*
	 * evalSup(double) method: gives the x value corresponding to the given probability
	 * Equivalent to the quantile function, inverse of the cdf
	 */
	double evalSup(double alpha);

	/*
	 * convolution(dist) method: calculate the convolution between the distribution given as a parameter and the current one
	 * mathematical definition on pdf: sum_h=0..t[p1(h)p2(t-h)], p1 and p2 being two pdf
	 * mathematical definition on cdf: sum_h=0..t[F1(h)p2(t-h)], F1 being a cdf and p2 being a pdf
	 */
	Distribution convolute(const Distribution& dist) const;

	/*
	 * aggregate(const uint32_t&, const double&) method: return the maximum value between a candidate double and the cdf value located at a given index
	 * aggregate distribution is such that for all t, F(t) = max{F1(t),F2(t)}, F being the agregated cdf, F1 and F2 being the input cdf
	 */
	double aggregate(const uint32_t& index, const double& candidateValue);

	/*
	 * aggregate(dist) method: return the aggregated distribution between the current one and dist
	 * aggregate distribution is such that for all t, F(t) = max{F1(t),F2(t)}, F being the agregated cdf, F1 and F2 being the input cdf
	 */
	Distribution aggregate(const Distribution& dist);

	/*
	 * dominates(dist) method: return true if current distribution cdf dominates dist cdf, for each support points (ie F1(t)<=F2(t) forall t in [0,Tmax], and at least one t such that F1(t)<F2(t) )
	 * corresponds to the first-order stochastic dominance (see Miller-Hooks & Mahmassani, 2003)
	 */
	bool dominates(const Distribution& dist);

	/*
	 * isLargerThan(dist) method: return true if current distribution cdf is larger than dist cdf, for each support points (ie F1(t)<=F2(t) forall t in [0,Tmax]), false otherwise
	 * based on dominates(dist) method, with a different equality case management
	 */
	bool isLargerThan(const Distribution& dist);

	/*
	 * << operator: return an outstream version of the distribution (printing purpose)
	 */
	friend std::ostream& operator<<(std::ostream& os, const Distribution& dist);

private:
	/*
	 * Attributes
	 */
	std::vector<uint32_t> _suppoints; // Support points of the discretized statistical distribution
	std::vector<double> _cdf; // Cumulative distribution function
	std::vector<double> _pdf; // Probability density function (actually a mass function, as the distribution is discretized)
};

#endif /* DATA_DISTRIBUTION_H_ */
