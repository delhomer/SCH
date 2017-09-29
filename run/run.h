/*
 * run.h
 *
 *  Created on: 2 févr. 2016
 *      Author: delhome
 */

#ifndef RUN_RUN_H_
#define RUN_RUN_H_

#include <cstdlib> // Commands 'EXIT_FAILURE' and 'EXIT_SUCCESS'
#include <iostream> // IO stream management
#include <vector>

#include "../misc.h" // Trace commands
#include "../data_io/configreader.h"
#include "../data_io/demandreader.h"
#include "../data_io/graphreader.h"
#include "../data_io/hierarchy_io.h"
#include "../data_io/specifreader.h"
#include "../data/graph/distribution.h"
#include "../data/graph/graph.h"
#include "../data/witnesssearch/heapitem.h"
#include "../data/witnesssearch/searchcontext.h"
#include "../data/witnesssearch/witnesssearch.h"
#include "../data/contraction/cachedwitness.h"
#include "../data/contraction/localthread.h"
#include "../data/contraction/ordering.h"
#include "../data/querying/schquery.h"
#include "../data/querying/spotar.h"

class Run {
    public:
        virtual ~Run() {};
        virtual int main(int argc, char *argv[]) = 0;
};



#endif /* RUN_RUN_H_ */
