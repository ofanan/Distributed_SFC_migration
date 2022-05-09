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

		// returns the scalar product of two vectors
		static vector<uint16_t> scalarProdcut (const vector<uint8_t> &vec1, const vector<uint16_t> &vec2) { 
			vector <uint16_t> res;
			for (uint8_t i(0); i<vec1.size(); i++) {
				res[i] = vec1[i] * vec2[i];
			}
			return res;
		}

};

#endif

