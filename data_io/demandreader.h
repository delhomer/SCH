/*
 * demandreader.h
 *
 *  Created on: 3 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_IO_DEMANDREADER_H_
#define DATA_IO_DEMANDREADER_H_

#include <fstream>
#include <sstream>
#include <cstdlib> // Commands atoi, atof, srand
#include <string>
#include <time.h> // Command time
#include <vector>

#include "../misc.h"

class Demands{

	/*
	 * Class Demand: structure aiming at describing one single demand (src->dest, for probability alpha or time timebudget)
	 */
	class Demand
	{
	public:
		/*
		 * Constructors
		 */
		Demand(): _start(0), _destination(0), _alpha(1), _timebudget(0){}
		Demand(const uint32_t& src, const uint32_t& dest, const double& a, const uint32_t& budg): _start(src), _destination(dest), _alpha(a), _timebudget(budg){}

		/*
		 * Getters
		 */
		Node_id getSrc() const { return _start; }
		Node_id getDest() const { return _destination; }
		double getAlpha() const { return _alpha; }
		uint32_t getBudget() const { return _timebudget; }

		/*
		 * toString() method: give demand parameters under the string format "{src,dest,alpha,budget}"
		 */
		std::string toString(){
			std::stringstream ss;
			ss << "{N" << _start << ",N" << _destination << ",p=" << _alpha << ",t=" << _timebudget << "}";
			return ss.str();
		}

	private:
		/*
		 * Parameters
		 */
		Node_id _start;
		Node_id _destination;
		double _alpha;
		uint32_t _timebudget;
	};

public:
	/*
	 * Constructors
	 */
	Demands(): _nbNodes(1), _tmax(0){}
	Demands(const uint32_t& nbNodes, const uint32_t& tmax): _nbNodes(nbNodes), _tmax(tmax){}
	Demands(const Demand& d):_nbNodes(1), _tmax(0){
		_demands.push_back(d);
	}
	Demands(Demands& dmds){
		_demands = dmds.getDemands();
		_nbNodes = dmds.getNbNodes();
		_tmax = dmds.getTmax();
	}
	Demands(const std::string& input_file_name, const uint32_t nbn): _nbNodes(nbn), _tmax(0){
		if (input_file_name == ""){
			ERROR("Empty input file name given.\n");
		}
//		STATUS("Reading demands file '" << input_file_name << "'...");
		std::ifstream input_demands_file(input_file_name);
		if ( ! input_demands_file.is_open() ){
//			CONTINUE_STATUS(" ABORT\n");
			ERROR("Unable to open file.\n");
		}
		std::string firstline;
		getline(input_demands_file,firstline);
		if ( firstline != "demands" ){
//			CONTINUE_STATUS(" ABORT\n");
			ERROR("Not a valid DEMANDS file (header format issue).\n");
		}
		uint32_t n_demands;
		input_demands_file >> n_demands;
		uint32_t start,destination,timebudget;
		double alpha;
		for ( size_t i = 0 ; i != n_demands ; ++i ){
			input_demands_file >> start;
			input_demands_file >> destination;
			input_demands_file >> alpha;
			input_demands_file >> timebudget;
			_demands.push_back( Demand(start,destination,alpha,timebudget) );
		}
		input_demands_file.close();
//		CONTINUE_STATUS(" OK\n");
	}
	Demands(const int& nbdemands, const uint32_t& nbn, const uint32_t& tmax): _nbNodes(nbn), _tmax(tmax){
		if( nbdemands == -1 ){
			TRACE("All OD pair generation...");
			for(uint32_t i(0) ; i < _nbNodes*_nbNodes ; ++i){
				uint32_t start( i / _nbNodes );
				uint32_t destination( i % _nbNodes );
				if(start == destination){
					continue;
				}
				double alpha( (rand() % 101) / 100.0 );
				uint32_t timebudget( rand() % _nbNodes );
				_demands.push_back( Demand(start,destination,alpha,timebudget) );
			}
		}
		else{
			TRACE("Generation of " << nbdemands << " random queries...");
			for(int i(1) ; i <= nbdemands ; ++i){
				uint32_t start( rand() % _nbNodes );
				uint32_t destination( rand() % _nbNodes );
				while(start == destination){
					destination = rand() % _nbNodes;
				}
				double alpha( (rand() % 101) / 100.0 );
				uint32_t timebudget( rand() % _nbNodes );
				_demands.push_back( Demand(start,destination,alpha,timebudget) );
			}
		}
	}

	/*
	 * Destructor
	 */
	~Demands(){	_demands.clear(); }

	/*
	 * Getters
	 */
	std::vector<Demand> getDemands() const { return _demands; }
	int getNbQueries() const { return _demands.size(); }
	std::pair<Node_id,Node_id> getOD(const uint32_t& index) const { return std::pair<Node_id,Node_id>( _demands[index].getSrc() , _demands[index].getDest() ); }
	uint32_t getNbNodes() const { return _nbNodes; }
	uint32_t getTmax() const { return _tmax; }
	uint32_t getNbDmds() const { return _demands.size(); }

	/*
	 * generate(const uint32_t) method: generate randomly as many queries (source->destination, under prob alpha, with time budget b) as indicated by the int parameter
	 */
	void generate(const uint32_t& nbQueries){
		// Write each query (source, destination, alpha, time budget)
		srand(time(NULL));
		for(unsigned int i(0) ; i < nbQueries ; ++i){
			uint32_t source( rand() % _nbNodes );
			uint32_t destination( rand() % _nbNodes );
			while(source == destination){
				destination = rand() % _nbNodes;
			}
			double alpha( (rand() % 101) / 100 );
			uint32_t timebudget( rand() % _tmax );
			_demands.push_back( Demand(source, destination, alpha, timebudget) );
		}
	}

	/*
	 * Demand serialization into a dmd file
	 */
	void serialize( const std::string& output_file_name ){
		// Writing process
		std::ofstream ofquery(output_file_name);
		// Write format tag
		ofquery << "demands\n";
		// Write number of queries
		ofquery << _demands.size() << "\n";
		// Write each query (source, destination, alpha, time budget)
		srand(time(NULL));
		for(auto query : _demands){
			ofquery << query.getSrc() << " " << query.getDest() << " " << query.getAlpha() << " " << query.getBudget() << "\n";
		}
		ofquery.close();
	}

	/*
	 * print() method: print a simple description of the demand list on the console
	 */
	void print(){
		if(_demands.size()>1){
			std::cout << "In this graph composed of " << _nbNodes << " nodes, the first and last queries are respectively:" << std::endl;
			std::cout << _demands.front().toString() << " --- " << _demands.back().toString() << std::endl;
		}else{
			std::cout << "In this graph composed of " << _nbNodes << " nodes, we test the following query:" << std::endl;
			std::cout << _demands.front().toString() << std::endl;
		}
	}

private:
	/*
	 * Parameters
	 */
	std::vector<Demand> _demands;
	uint32_t _nbNodes;
	uint32_t _tmax;
};

#endif /* DATA_IO_DEMANDREADER_H_ */
