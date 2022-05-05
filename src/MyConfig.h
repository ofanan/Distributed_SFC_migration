#ifndef MYCONFIG_H
#define MYCONFIG_H
#include <stdio.h>
#include <stdint.h>

#include <vector>
#include <set>
#include <algorithm>
#include <unordered_set>

#include <omnetpp.h>
#include "Parameters.h"

using namespace omnetpp;
using namespace std;

class MyConfig {
	public:
	
		// Print vector in a formated fashion
		static void printVec (ofstream &outFile, vector<int16_t> vec) {
			for (vector <int16_t>::const_iterator it (vec.begin()); it <= vec.end(); it++){
				outFile << *it << ";";
			}
/*		  for_each(vec.begin(), vec.end(),[](int number){outFile << number << ";";});*/
		  outFile << endl;
		};
		
		// Print vector in a formated fashion
/*		static void printVec (ofstream &outFile, vector<int32_t> vec) {*/
/*		  for_each(vec.begin(), vec.end(),[](int number){outFile << number << ";";});*/
/*		  outFile << endl;*/
/*		};*/
};

#endif

