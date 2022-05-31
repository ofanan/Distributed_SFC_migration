// Configuration parameters, writing to log files, and accessory functions.

#ifndef MY_CONFIG_H
#define MY_CONFIG_H
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <regex>
#include <utility>
#include <cstdlib>

#include <vector>
#include <set>
#include <algorithm>
#include <unordered_set>

#include "Chain.h"
#include "SetsOfChains.h"

using namespace omnetpp;
using namespace std;

class Chain;
class ChainHash;
typedef int32_t ChainId_t;

const int16_t nonAugmentedCpuAtLvl[] = {1,1,1};

const int16_t chainMigCost = 600;

const int32_t DUMMY  		= -1;
const int16_t UNPLACED  = -1;
extern const int8_t  UNPLACED_;

const bool SYNC  = true;
const bool ASYNC = false;

const int BASIC_LOG=1, DETAILED_LOG=2, VERY_DETAILED_LOG = 3;

const float RT_chain_pr = 0.0; // prob' that a new chain is an RT chain


class MyConfig { 

		static const uint16_t bufSize = 128;
		static char buf[bufSize];

	public:
	
		static unordered_set <Chain, ChainHash> allChains; // All the currently active chains. 
 		static string LogFileName;
    static ofstream logFile;
		const static bool mode=SYNC;
		static const int DEBUG_LVL=1, LOG_LVL=DETAILED_LOG;

		//Init
		static void openFiles ();
		
		//print
		static void printAllChains ();
		static void printAllChainsPoas ();
		static void printToLog (char* buf); 
		static void printToLog (string str);
		static void printToLog (int d); // print "d,", where d is the integer, to the log file
		static void printToLog (vector <Chain> vec); // print all the IDs of the chains in the vec.
		static void printToLog (vector <uint16_t> vec); // print a vec of integers to the log file
		static void printToLog (vector <ChainId_t> vec); // print a vec of ChainId_t to the log file
		static void printToLog (unordered_set <ChainId_t> set2print);
		static void printToLog (list <Chain> list2print, bool printLvl=true);
/*		static void printToLog (SetOfChainsOrderedByDecCpuUsage setOfChains, bool printS_u=false);*/
		static void printToLog (unordered_set <Chain, ChainHash> set2print);
		static void printSuToLog (Chain chain);
		
		// Other accessories funcs'
		static ofstream getLogFile ();
		static vector<uint16_t> scalarProdcut (const vector<uint16_t> &vec1, const vector<uint16_t> &vec2); // returns the scalar product of two vectors
		static bool eraseKeyFromSet (unordered_set <ChainId_t> &set, uint16_t id);
/*		static bool eraseKeyFromSet (unordered_set <ChainId_t> set, uint16_t id); // erase the given key from the given set. Returns true iff the requested key was indeed found in the set*/
};

#endif


