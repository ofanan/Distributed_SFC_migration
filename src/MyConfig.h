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
/*
		static int dotProdcut (uint8_t vec1[], uint16_t vec2[]) { //(vector<uint8_t> vec1, vector<uint16_t> vec2) {
			if (vec1.size() != vec2.size()) {
				return -1;
			}
			int res = 0;
			for (uint8_t i(0); i<vec1.size(); i++) {
				res += vec1[i] * vec2[i];
			}
			return res;
		}
*/	
	/*
		// Print vector in a formated fashion
		static void printVec (ofstream &outFile, vector<int16_t> vec) {
			for (vector <int16_t>::const_iterator it (vec.begin()); it <= vec.end(); it++){
				outFile << *it << ";";
			}
//		  for_each(vec.begin(), vec.end(),[](int number){outFile << number << ";";});
		  outFile << endl;
		};
		
		// Print vector in a formated fashion
		static void printVec (ofstream &outFile, vector<int32_t> vec) {
			for (vector <int32_t>::const_iterator it (vec.begin()); it <= vec.end(); it++){
				outFile << *it << ";";
			}
		  outFile << endl;
		};
		*/
};

#endif

