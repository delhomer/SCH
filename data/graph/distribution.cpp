/*
 * distribution.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: delhome
 */

#include "distribution.h"


/*
 * Constructors
 */
Distribution::Distribution(){
	_suppoints.push_back(0);
	_cdf.push_back(1);
	_pdf.push_back(1);
}
Distribution::Distribution(const int& size): _suppoints(size,0), _cdf(size,0), _pdf(size,0){
	std::iota(_suppoints.begin(), _suppoints.end(), 0);
	_pdf[size-1] = 1;
	_cdf[size-1] = 1;
}
Distribution::Distribution(const int& size, const int& delta): _suppoints(size,0), _cdf(size,0), _pdf(size,0){
	std::iota(_suppoints.begin(), _suppoints.end(), 0);
	std::transform(_suppoints.begin(),_suppoints.end(),_suppoints.begin(),std::bind2nd( std::multiplies<double>(), delta) );
	_pdf[size-1] = 1;
	_cdf[size-1] = 1;
}
Distribution::Distribution(const int& size, const int& delta, const bool& infinite):_suppoints(size,0),_cdf(size,1),_pdf(size,0){
	if(infinite){
		Distribution(size,delta);
	}
	else{
		std::iota(_suppoints.begin(), _suppoints.end(), 0);
		std::transform(_suppoints.begin(),_suppoints.end(),_suppoints.begin(),std::bind2nd( std::multiplies<double>(), delta) );
		_pdf[0] = 1;
	}
}
Distribution::Distribution(const std::vector<uint32_t>& x, const std::vector<double>& p, const std::vector<double>& u){
	_suppoints = x;
	_pdf = p;
	_cdf = u;
}
Distribution::Distribution(const std::string& filename){
	if (filename == "")
	{
		ERROR("Empty input file name given.\n");
	}
	STATUS("Reading CDF file '" << filename << "'...");
	std::ifstream input_cdf(filename);
	if ( ! input_cdf.is_open() )
	{
		CONTINUE_STATUS(" ABORT\n");
		ERROR("Unable to open file '" << filename << "'\n");
	}
	uint32_t suppoint(0);
	double alpha(0);
	while( !input_cdf.eof() ){
		input_cdf >> suppoint;
		_suppoints.push_back( suppoint );
		input_cdf >> alpha;
		_pdf.push_back( alpha );
		_cdf.push_back( alpha );
	}
	std::partial_sum(_pdf.begin(), _pdf.end(), _cdf.begin());
	CONTINUE_STATUS("OK.\n");
	input_cdf.close();
}

/*
 * Destructor
 */
Distribution::~Distribution(){
	_suppoints.clear();
	_cdf.clear();
	_pdf.clear();
}

/*
 * Getters
 */
std::vector<uint32_t> Distribution::getSup() const { return _suppoints; }
std::vector<double> Distribution::getCdf() const { return _cdf; }
std::vector<double> Distribution::getPdf() const { return _pdf; }
uint32_t Distribution::getSupT(const uint32_t& t) const { return _suppoints[t]; }
double Distribution::getCdfT(const uint32_t& t) const { return _cdf[t]; }
double Distribution::getPdfT(const uint32_t& t) const { return _pdf[t]; }

/*
 * Setters
 */
void Distribution::setSup(std::vector<uint32_t> newSup){ _suppoints = newSup; }
void Distribution::setPdf(std::vector<double> newPdf){ _pdf = newPdf; }
void Distribution::setCdf(std::vector<double> newCdf){ _cdf = newCdf; }
void Distribution::setSupT(uint32_t index, uint32_t newValue){
	assert((index>=0)&&(index<_suppoints.size()));
	_suppoints[index] = newValue;
}
void Distribution::setPdfT(uint32_t index, double newValue){
	assert( (index>=0) && (index<_pdf.size()) );
	if(!le(0,newValue)){
		ERROR("Negative pdf error: Pdf value provided for index t=" << index << " is not comprised between 0 and 1! (" << newValue << ")" << "\n");
	}
	if(!le(newValue,1)){
		ERROR("Larger than 1 pdf error: Pdf value provided for index t=" << index << " is not comprised between 0 and 1! (" << newValue << ")" << "\n");
	}
//	assert( le(0,newValue) && le(newValue,1) );
	_pdf[index] = newValue;
}
void Distribution::setCdfT(uint32_t index, double newValue){
	assert((index>=0)&&(index<_cdf.size()));
	if(!le(0,newValue)){
		ERROR("Negative pdf error: Pdf value provided for index t=" << index << " is not comprised between 0 and 1! (" << newValue << ")" << "\n");
	}
	if(!le(newValue,1)){
		ERROR("Larger than 1 pdf error: Pdf value provided for index t=" << index << " is not comprised between 0 and 1! (" << newValue << ")" << "\n");
	}
//	assert(le(0,newValue)&&le(newValue,1));
	_cdf[index] = newValue;
}

/*
 * isInfinite() method: check if the distribution is "infinite", that means the only possible value is the maximum of the admitted range [0;Tmax], ie cdf(t)=1 if t=Tmax, cdf(t)=0 otherwise
 */
bool Distribution::isInfinite(){
	auto it = std::find_if(_cdf.begin(), _cdf.end(), [](double& a)->bool { /*std::cout << a << "-";*/ return eq(a,1); } );
	//		TRACE("it: " << *it << " --- _cdf.size() = " << _cdf.size() );
	return it == _cdf.end()-1 && _cdf.size() > 1;// && (*_cdf.end() == 1);
}

/*
 * reset() method: reset the distribution
 */
void Distribution::reset(){
	_suppoints.clear();
	_suppoints.push_back(0);
	_pdf.clear();
	_pdf.push_back(1);
	_cdf.clear();
	_cdf.push_back(1);
}

/*
 * makeInfinite() method: set the distribution at the "infinite" status
 * Here infinite means the same size, but the only support point with non-nul probability is the maximal one
 */
void Distribution::makeInfinite(){
	std::fill(_pdf.begin(), _pdf.end(), 0 );
	std::fill(_cdf.begin(), _cdf.end(), 0 );
	_pdf.back() = 1;
	_cdf.back() = 1;
}

/*
 * getSize() method: return the number of support points
 */
uint32_t Distribution::getSize() const{ return _suppoints.size(); }

/*
 * getDelta() method: return the difference between each support points (defined as a constant value)
 */
uint32_t Distribution::getDelta() const{
	if(_suppoints.size()>1)
		return _suppoints[1]-_suppoints[0];
	else
		return 0;
}

/*
 * getSupI(uint32_t) method: return the i^th support point
 */
uint32_t Distribution::getSupI(uint32_t index) const{
	assert((index>=0)&&(index<_suppoints.size()));
	return _suppoints[index];
}

/*
 * getProbI(double) method: return the i^th step of the cdf
 */
double Distribution::getPdfI(uint32_t index) const{
	assert((index>=0)&&(index<_pdf.size()));
	return _pdf[index];
}

/*
 * getCumI(double) method: return the i^th step of the cdf
 */
double Distribution::getCdfI(uint32_t index) const{
	assert((index>=0)&&(index<_cdf.size()));
	return _cdf[index];
}

/*
 * Cdf(uint32_t) method: gives the probability of having a value less than the parameter
 */
double Distribution::Cdf(uint32_t t) const{
	assert( t>=0 );
	uint32_t delta = getDelta();
	if(t >= _suppoints.size()*delta )
		return 1;
	else
		return _cdf[ t / delta ];
}

/*
 * Pdf(uint32_t) method: gives the probability of having a value equal to parameter
 */
double Distribution::Pdf(uint32_t t) const{
	assert( t>=0 );
	uint32_t delta = getDelta();
	return ( t%delta == 0 && t>=delta*_suppoints.size() )?_pdf[ t / delta ]:0;
}

/*
 * min() method: return the minimum possible realization
 */
uint32_t Distribution::min() const{
	std::vector<double>::const_iterator itcdf = std::find_if(_cdf.begin(), _cdf.end(), [](const double& value) { return gt(value,0); });
//	TRACE("Min element index: " << itcdf - _cdf.begin() << " (total elements:" << _cdf.size() << ") ==> MIN=" << _suppoints[itcdf-_cdf.begin()] );
	return _suppoints[itcdf - _cdf.begin()];
}

/*
 * max() method: return the maximum possible realization
 */
uint32_t Distribution::max() const{
	std::vector<double>::const_iterator itcdf = std::find_if(_cdf.begin(), _cdf.end(), [](const double& value) { return eq( value , 1.0 ); });
//	TRACE("Max element index: " << itcdf - _cdf.begin() << " (total elements:" << _cdf.size() << ") ==> MAX=" << _suppoints[itcdf-_cdf.begin()]);
	return _suppoints[itcdf - _cdf.begin()];
}

/*
 * range() method: return the definition range of the distribution, namely max()-min()
 */
uint32_t Distribution::range() const{
	std::vector<double>::const_iterator itcdfmin = std::find_if(_cdf.begin(), _cdf.end(), [](const double& value) { return gt(value,0); });
	std::vector<double>::const_iterator itcdfmax = std::find_if(itcdfmin, _cdf.end(), [](const double& value) { return eq(value,1); });
//	TRACE("Min element index: " << itcdfmin - _cdf.begin() << " -- Max element index: " << itcdfmax - _cdf.begin() << " (total elements:"
//			<< _cdf.size() << ") ==> MIN=" << _suppoints[itcdfmin-_cdf.begin()]  << " MAX=" << _suppoints[itcdfmax-_cdf.begin()]);
	return ( _suppoints[itcdfmax - _cdf.begin()] - _suppoints[itcdfmin - _cdf.begin()] ) ;
}

/*
 * esp() method: return the esperance of the distribution
 * E(T) = sum_t=0->Tmax_{t*p(t)}
 */
double Distribution::esp() const{
	return std::inner_product( _suppoints.begin() , _suppoints.end() , _pdf.begin() , 0.0);
}

/*
 * evalSup(double) method: gives the x value corresponding to the given probability
 * Equivalent to the quantile function, inverse of the cdf
 */
double Distribution::evalSup(double alpha){
	assert((alpha<=1+EPSILON) && (alpha>=-EPSILON));
	std::vector<double>::iterator italpha = _cdf.begin();
	while( (EPSILON < alpha-*italpha) && (italpha < _cdf.end()-1) ){
		++italpha;
	}
	return _suppoints[ italpha - _cdf.begin() ];
}

/*
 * convolution(dist) method: calculate the convolution between the distribution given as a parameter and the current one
 * mathematical definition on pdf: sum_h=0..t[p1(h)p2(t-h)], p1 and p2 being two pdf
 * mathematical definition on cdf: sum_h=0..t[F1(h)p2(t-h)], F1 being a cdf and p2 being a pdf
 */
Distribution Distribution::convolute(const Distribution& dist) const {
//	TRACE("Convoluted: " << *this);
//	TRACE("Convoluant: " << dist);
	std::vector<double> pdf, cdf;
	std::vector<uint32_t>::const_iterator itt = _suppoints.begin();
	std::vector<double> convolutedPdf = dist.getPdf();
	for(uint32_t t(0) ; t < getSize() ; ++t){
		double probsumPdf(0), probsumCdf(0);
		for(uint32_t tau(0) ; tau <= t ; ++tau){
			probsumPdf += _pdf[tau] * dist.getPdfT(t-tau);
			probsumCdf += _cdf[tau] * dist.getPdfT(t-tau);
		}
		pdf.push_back(probsumPdf);
		cdf.push_back(probsumCdf);
	}
	if( !eq( cdf.back() , 1.0 ) ){
//		TRACEF(Distribution(_suppoints,pdf,cdf) << "\nCDF COMPLETION: " << "[" << pdf.back() << "-" << cdf.back() << "] --> <" << cdf[ getSize()-1 ] << "/" << cdf[ getSize()-2 ] << ">");
		cdf.back() = 1.0;
		pdf.back() = cdf.back() - cdf[ getSize()-2 ];
//		CONTINUE_STATUS(" [" << pdf.back() << "-" << cdf.back() << "]\n" << Distribution(_suppoints,pdf,cdf));
	}
	if( !eq(cdf.back(),1.0) ){
		TRACE("Convolution result: " << Distribution(_suppoints,pdf,cdf));
	}
	assert( eq(cdf.back(),1.0) ); // Catch cases where convoluted distribution can be out of the period definition
	return Distribution(_suppoints,pdf,cdf);
}

/*
 * aggregate(const uint32_t&, const double&) method: return the maximum value between a candidate double and the cdf value located at a given index
 * aggregate distribution is such that for all t, F(t) = max{F1(t),F2(t)}, F being the agregated cdf, F1 and F2 being the input cdf
 */
double Distribution::aggregate(const uint32_t& index, const double& candidateValue){
	double bestValue( _cdf[index] );
	if( lt(bestValue,candidateValue) ){
		return candidateValue;
	}
	return bestValue;
}

/*
 * aggregate(const Distribution&) method: modify current distribution to get the aggregated distribution between it and the given one
 * aggregate distribution is such that for all t, F(t) = max{F1(t),F2(t)}, F being the agregated cdf, F1 and F2 being the input cdf
 */
Distribution Distribution::aggregate(const Distribution& dist){
	std::vector<double> pdf, cdf;
	setCdfT( 0 , std::max(_cdf[0],dist.getCdfI(0)) );
	setPdfT( 0 , getCdfI(0) );
	std::vector<double>::iterator itf1, itf2;
	for(itf1=_cdf.begin()+1 ; itf1 < _cdf.end() ; ++itf1){
		setCdfT( itf1 - _cdf.begin() , std::max( _cdf[itf1 - _cdf.begin()] , dist.getCdfI(itf1 - _cdf.begin()) ) );
		setPdfT( itf1 - _cdf.begin() , getCdfI(itf1 - _cdf.begin()) - getCdfI(itf1 - _cdf.begin() - 1) );
	}
	return *this;
}

/*
 * dominates(dist) method: return true if current distribution cdf dominates dist cdf, for each support points (ie F1(t)<=F2(t) forall t in [0,Tmax], and at least one t such that F1(t)<F2(t) )
 * corresponds to the first-order stochastic dominance (see Miller-Hooks & Mahmassani, 2003)
 */
bool Distribution::dominates(const Distribution& dist){
	// By default, consider than current distribution do not dominate the candidate
	bool response = false;
	std::vector<double>::iterator italt = dist.getCdf().begin();
	// Comparison over all cdf values
	for(auto &it: _cdf){
		// If there is an index for which candidate cdf is strictly larger than current cdf, candidate is not dominated
		if( !le(*italt,it) ){
			return false;
		}
		// If there is an index such that current cdf is strictly larger than the candidate cdf, the former can dominate the latter (equality case management)
		if( !le(it,*italt) ){
			response = true;
		}
		// If both cdf are equal to 1, the algorithm can stop, we consider that the candidate is dominated (there is no index for which candidate cdf is larger)
		if( eq(it,1) && eq(*italt,1)){
			return response;
		}
		++italt;
	}
	// End of cdf checking, there is no index such that candidate cdf is larger than current cdf: the candidate distribution is dominated
	return response;
}

/*
 * isLargerThan(dist) method: return true if current distribution cdf is larger than dist cdf, for each support points (ie F1(t)<=F2(t) forall t in [0,Tmax]), false otherwise
 * based on dominates(dist) method, with a different equality case management
 */
bool Distribution::isLargerThan(const Distribution& dist){
	std::vector<double>::iterator italt = dist.getCdf().begin();
	// Comparison over all cdf values
	for(auto &it: _cdf){
		// If there is an index for which candidate cdf is strictly larger than current cdf, candidate is not dominated
		if( !le(*italt,it) ){
			return false;
		}
		// If both cdf are equal to 1, the algorithm can stop, we consider that the candidate is dominated (there is no index for which candidate cdf is larger)
		if( eq(it,1) && eq(*italt,1)){
			return true;
		}
		++italt;
	}
	// End of cdf checking, there is no index such that candidate cdf is larger than current cdf: the candidate distribution is dominated
	return true;
}

/*
 * << operator: return an outstream version of the distribution (printing purpose)
 */
std::ostream& operator<<(std::ostream& os, const Distribution& dist){
	std::vector<uint32_t> sup = dist._suppoints;
	std::vector<double> prob = dist._pdf;
	std::vector<double> cum = dist._cdf;
	std::vector<uint32_t>::iterator itt = sup.begin();
	std::vector<double>::iterator itm = prob.begin();
	std::vector<double>::iterator itf = cum.begin();
	for( ; itt < sup.end() ; ++itt,++itm,++itf ){
		os << "(" << *itt << ";" << *itm << ";" << *itf << ") ";
	}
	return os;
}

