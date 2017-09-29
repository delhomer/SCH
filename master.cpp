/*
 * master.cpp
 *
 *  Created on: 2 févr. 2016
 *      Author: delhome
 */

#include <unistd.h>

#include "misc.h"
#include "run/schprocedure.h"
#include "run/codetester.h"
#include "run/fullanalysis.h"
#include "run/querygen.h"

int main(int argc, char** argv){
	int result(0);
	int opt = getopt(argc, argv, "dfpt");

	Run* run = NULL;

	switch(opt){
	case 'd':
		MARK("### Demand generation ###");
		run = new QueryGenerator();
		break;
	case 'f':
		MARK("### Full list of instance testing ###");
		run = new FullAnalysis();
		break;
	case 'p':
		MARK("### Stochastic Contraction Hierarchy Procedure ###");
		run = new SCHProcedure();
		break;
		//	case 'g':
		//		std::cout << "### Graph generation ###" << std::endl;
		//		run = new GraphGen();
		//		break;
		//	case 'h':
		//		std::cout << "### Hierarchy optimization ###" << std::endl;
		//		run = new tch_run::Tch_HierarchyOptimization();
		//		break;
		//	case 'm':
		//		std::cout << "### Global hierarchy parametrization evaluation ###" << std::endl;
		//		run = new tch_run::Tch_GlobalHierarchyEvaluation();
		//		break;
		//	case 'o':
		//		std::cout << "### Node ordering ###" << std::endl;
		//		run = new tch_run::Tch_NodeOrdering();
		//		break;
		//	case 'q':
		//		std::cout << "### Query Generation ###" << std::endl;
		//		run = new tch_run::Tch_QueryGen();
		//		break;
		//	case 's':
		//		std::cout << "### Query solving ###" << std::endl;
		//		run = new tch_run::Tch_Query();
		//		break;
	case 't':
		MARK("### Code Tester ###");
		run = new CodeTester();
		break;
	default:
		MARK("### No argument... Test procedures begin!");
	}
	if (run != NULL) {
		result = run->main(argc, argv);
		delete run;
	} else {
		MARK("For help see documentation");
	}
	return result;
}
