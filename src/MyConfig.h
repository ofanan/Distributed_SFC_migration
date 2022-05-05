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
		static void printVec (ofstream &outFile, vector<int16_t> vec) {
			for(auto it = begin(vec); it != end(vec); it++) {
		  	outFile << *it << ",";
			}
			outFile << *(end(vec)) << ";" ;
		};
		
		static void printVec (ofstream &outFile, vector<int32_t> vec) {
			for(auto it = begin(vec); it != end(vec); it++) {
		  	outFile << *it << ",";
			}
			outFile << *(end(vec)) << ";" ;
		};
};

#endif

