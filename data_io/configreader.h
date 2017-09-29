/*
 * configreader.h
 *
 *  Created on: 3 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_IO_CONFIGREADER_H_
#define DATA_IO_CONFIGREADER_H_

#include <fstream>
#include <sstream>
#include <cstdlib> // Commands atoi, atof, srand
#include <set>
#include <string>
#include <time.h> // Command time
#include <vector>

#include "../misc.h"

class Configs{

public:
	/*
	 * Class config: nested class describing a single configuration
	 */
	class Config
	{
	private:
		/*
		 * Parameters
		 */
		double param_eq;
		double param_ssd;
		double param_oeq;
		double param_cq;
		int lastmove;

	public:
		/*
		 * Constructors
		 */
		Config():param_eq(0),param_ssd(0),param_oeq(0),param_cq(0),lastmove(-1){}
		Config(const double& a,const double& b,const double& c,const double& d, const int& m):param_eq(a),param_ssd(b),param_oeq(c),param_cq(d), lastmove(m){}
		Config(const Config& cnfg):param_eq(cnfg.param_eq),param_ssd(cnfg.param_ssd),param_oeq(cnfg.param_oeq),param_cq(cnfg.param_cq),lastmove(cnfg.lastmove){}
		/*
		 * Getters
		 */
		double getParamEq() const { return param_eq; }
		double getParamSsd() const { return param_ssd; }
		double getParamOeq() const { return param_oeq; }
		double getParamCq() const { return param_cq; }

		/*
		 * Setters
		 */
		void setParamEq(const double& eq){ param_eq = eq; }
		void setParamSsd(const double& ssd){ param_ssd = ssd; }
		void setParamOeq(const double& oeq){ param_oeq = oeq; }
		void setParamCq(const double& cq){ param_cq = cq; }

		/*
		 * toString() method: print the configuration under the format {p1,p2,p3,p4}
		 */
		std::string toString(){
			std::stringstream ssconfig;
			ssconfig << "{" << param_eq << "," << param_ssd << "," << param_oeq << "," << param_cq << "}";
			return ssconfig.str();
		}

		/*
		 * operator<< : print a simple description of the config on the console
		 */
		friend std::ostream& operator<<(std::ostream& os, const Config& s){
			os << "{" << s.param_eq << "," << s.param_ssd << "," << s.param_oeq << "," << s.param_cq << "}";
			return os;
		}

		/*
		 * neighborhood(int) method: select a neighbor configuration by incrementing/decrementing the value of one parameter
		 * the set of configurations is such that the sum of coefficient must be equal to 1000, and every coefficients are positive
		 */
		Config neighborhood(const int& lastmoveid){
			int params[4] = {(int)param_eq,(int)param_ssd,(int)param_oeq,(int)param_cq};
			int firstindex( -1 ), secondindex( -1 );
			srand( time(0) );
			Move move;
			int moveid( -1 );
			Move lastmove( lastmoveid );
			do{
				moveid = rand() % 12 ;
				move.setIndices(moveid);
				while( move.isOpposite( lastmove ) ){
					moveid = rand() % 12 ;
					move.setIndices(moveid);
				}
				firstindex = move.getfirstIndex();
				secondindex = move.getsecondIndex();
			}while( (params[firstindex] == 0 || params[secondindex] == 1000) ); // Avoid negative or too large indices
			// Modify indices
			--params[firstindex];
			++params[secondindex];
			TRACE("Final move: " << move.print() << " --> config = {" << params[0] << "," << params[1] << "," << params[2] << "," << params[3] << "}");
			return Config(params[0],params[1],params[2],params[3],moveid);
		}
		/*
		 * diversify() method: build a new configuration starting from the model of the current configuration
		 * similar to neighborhood, but with a larger shift (by default +- 100 units)
		 */
		Config diversify(/*int divquantity*/){ // Shift coefficient
			int params[4] = {(int)param_eq,(int)param_ssd,(int)param_oeq,(int)param_cq};
			int firstindex( -1 ), secondindex( -1 );
			srand( time(0) );
			Move move;
			int moveid( -1 );
			do{
				moveid = rand() % 12 ;
				move.setIndices(moveid);
				firstindex = move.getfirstIndex();
				secondindex = move.getsecondIndex();
			}while( (params[firstindex] <= 100 || params[secondindex] >= 900) ); // Avoid negative or too large indices
			std::cout << params[firstindex] << "-" << params[secondindex] << std::endl;
			params[firstindex] = params[firstindex] - 100;
			params[secondindex] = params[secondindex] + 100;
			std::cout << params[firstindex] << "-" << params[secondindex] << std::endl;
			return Config(params[0],params[1],params[2],params[3],-1);
		}
		/*
		 * Class Move: nested class representing the configuration moves:
		 * One coefficient has to be decreased, and one must be increased, over 4 coefficients => 6 combinations * 2 directions = 12 possible moves
		 */
		class Move{
		private:
			/*
			 * Parameters
			 */
			int firstIndex,secondIndex;
		public:
			/*
			 * Constructors
			 */
			Move():firstIndex(0),secondIndex(1){}
			Move(const int& a, const int&b):firstIndex(a),secondIndex(b){}
			Move(const int& a):firstIndex(a/3){
				int amod( a % 3 );
				if(amod == 0){
					secondIndex = firstIndex == 0? 1 : 0;
				}else if(amod == 1){
					secondIndex = firstIndex < 2? 2 : 1;
				}else{
					secondIndex = firstIndex == 3? 2 : 3;
				}
			}
			/*
			 * Getters
			 */
			int getfirstIndex() const { return firstIndex;	}
			int getsecondIndex() const { return secondIndex; }
			/*
			 * Setter
			 */
			void setIndices(const int& a){
				firstIndex = a/3;
				int amod( a % 3 );
				if(amod == 0){
					secondIndex = firstIndex == 0? 1 : 0;
				}else if(amod == 1){
					secondIndex = firstIndex < 2? 2 : 1;
				}else{
					secondIndex = firstIndex == 3? 2 : 3;
				}
			}
			/*
			 * identifier() method: find the move id, starting from the ids of decreased and increased coefficients
			 */
			int identifier(){
				std::set<int> indices = {0,1,2,3};
				indices.erase( indices.find(firstIndex) );
				int i2( *indices.find(secondIndex) );
				return firstIndex * 3 + i2 ;
			}
			/*
			 * oppositeMove(): return the opposite move of the current one (the decreased coefficient is now increased, and vice-versa)
			 */
			Move oppositeMove(){
				return Move( secondIndex , firstIndex );
			}

			/*
			 * isOpposite(Move): return true if m2 is the current move opposite
			 */
			bool isOpposite(const Move& m2){
				return secondIndex == m2.firstIndex && firstIndex == m2.secondIndex ;
			}
			/*
			 * print() method: print on the console a short description of the current move
			 */
			std::string print(){
				std::stringstream resultss;
				resultss << "Index " << firstIndex << " is decreased, " << secondIndex << " is increased!";
				return resultss.str();
			}
		};
	};

	Configs(){}
	Configs(const std::vector<Config>& cnfgs){
		_configs = cnfgs;
	}
	Configs(const std::string& config_file_name){
		if (config_file_name == ""){
			ERROR("Empty config file name given.\n");
		}
//		STATUS("Reading config file '" << config_file_name << "'...");
		std::ifstream configs_file(config_file_name);
		configs_file.precision(15);
		if ( ! configs_file.is_open() ){
//			CONTINUE_STATUS(" ABORT\n");
			ERROR("Unable to open config file.\n");
		}
		std::string paramline;
		while( getline(configs_file,paramline) ){
			std::stringstream config_stream(paramline);
			double coef1, coef2, coef3, coef4;
			config_stream >> coef1;
			config_stream >> coef2;
			config_stream >> coef3;
			config_stream >> coef4;
			_configs.push_back( Config(coef1,coef2,coef3,coef4,-1) );
		}
		configs_file.close();
//		CONTINUE_STATUS(" OK\n");
	}

	/*
	 * getFirst() method: return the first tuple of the config list
	 */
	Config getFirst() const { return _configs.front(); }

	/*
	 * operator<< : print a simple description of the config list on the console
	 */
	friend std::ostream& operator<<(std::ostream& os, const Configs& s){
		if(s._configs.size()>1){
			os << "Here are some examples of used configs:" << std::endl;
			os << s._configs.front() << " --- " << s._configs.back() << std::endl;
		}else{
			os << "Used config:" << std::endl;
			os << s._configs.front() << std::endl;
		}
		return os;
	}

private:
	std::vector<Config> _configs;

};

#endif /* DATA_IO_CONFIGREADER_H_ */
