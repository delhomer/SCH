/*
 * shortcut_io.h
 *
 *  Created on: 26 févr. 2016
 *      Author: delhome
 */

#ifndef DATA_IO_EDGE_IO_H_
#define DATA_IO_EDGE_IO_H_

#include "../misc.h"

class EdgeIO{
public:
	/*
	 * Constructors
	 */
	EdgeIO(): _edgeFile(){}
	EdgeIO(const std::string& filename, const std::vector<Edge>& shortcuts){
		_edgeFile = filename;
		_edges = shortcuts;
	}

	/*
	 * read() method: read shortcut stored in _shortcutFile
	 * Need a specification as an input
	 */
	void read(const Specif& specif){
		if (_edgeFile == ""){
			ERROR("Empty shortcut file name given.\n");
		}
//		STATUS("Reading shortcut file '" << _edgeFile << "'...");
		std::ifstream shortcutStream(_edgeFile);
		if ( ! shortcutStream.is_open() ){
//			CONTINUE_STATUS(" ABORT\n");
			ERROR("Unable to open hierarchy file.\n");
		}
		uint32_t shortcutCounter(0);
		while ( ! shortcutStream.eof() && shortcutCounter < specif.getNbEdges() ){
			++shortcutCounter;
			Node_id src, tgt;
			shortcutStream >> src;
			shortcutStream >> tgt;
			Node_id midnod;
			shortcutStream >> midnod;
//			TRACE("N" << src << "->N" << tgt << "(-N" << midnod << ")-");
//			std::vector<Node_id> midnods;
//			for ( uint32_t i = 0; i <= specif.getNbPts() ; i++ ){
//				Node_id midnod_id;
//				shortcutStream >> midnod_id;
//				midnods.push_back( midnod_id );
//			}
			std::vector<uint32_t> t;
			std::vector<double> pmf,cdf;
			for ( uint32_t i = 0; i <= specif.getNbPts() ; i++ ){
				uint32_t x(i * specif.getDelta() );
				t.push_back( x );
				double proba;
				shortcutStream >> proba;
				pmf.push_back( proba );
				cdf.push_back( proba );
				if ( x < 0 || x > specif.getNbPts() * specif.getDelta() ){
					CONTINUE_STATUS(" ABORT\n");
					ERROR("Edge file corrupted: x-value not in [0;Tmax*deltat].\n");
					_edges.clear();
				}
				if ( proba < 0 || proba > 1){
					CONTINUE_STATUS(" ABORT\n");
					ERROR("Edge file corrupted: y-value not in [0;1].\n");
					_edges.clear();
				}
			}
			std::partial_sum(pmf.begin(),pmf.end(),cdf.begin());
			if ( neq(cdf.back(),1)  ){ // abs(std::accumulate(pmf.begin(),pmf.end(),0)-1) < EPSILON
//				CONTINUE_STATUS(" ABORT\n");
				ERROR("Edge file corrupted: the total of probabilities is not equal to 1. (Fmax=" << cdf.back() << ")\n");
				_edges.clear();
				break;
			}
			Distribution dist = Distribution(t,pmf,cdf);
			_edges.push_back( std::move( Edge( true , src , tgt , dist, midnod ) ) );
//			TRACE("Last read edge: " << _edges.back());
		}
		shortcutStream.close();
//		CONTINUE_STATUS(" OK\n");
	}

	/*
	 * write() method:
	 */
	void write(){
//		STATUS("Writing shortcut file '" << _edgeFile << "'...");
		std::ofstream shortcutOut;
		shortcutOut.open(_edgeFile);
		shortcutOut << std::fixed << std::setprecision(9);
		std::vector<Edge>::iterator its = _edges.begin();
		for( ; its < _edges.end() ; ++its ){
//			TRACE("Write edge: " << *its);
			shortcutOut << its->getOrigin() << " " << its->getDestination() << " ";
			Node_id midnod = its->getMiddleNode();
			shortcutOut << midnod << " ";
			Distribution shortcutDist( its->getWeight() );
			for( uint32_t sup = 0 ; sup < shortcutDist.getSize() ; ++sup ){
				shortcutOut << shortcutDist.getPdfI(sup) << " ";
			}
			shortcutOut << "\n";
		}
		shortcutOut.close();
//		CONTINUE_STATUS(" OK\n");
	}

private:
	/*
	 * Parameters
	 */
	std::vector<Edge> _edges;
	std::string _edgeFile;
};



#endif /* DATA_IO_EDGE_IO_H_ */
