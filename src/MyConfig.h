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

#include "MyTypes.h"
#include "Chain.h"
#include "ChainsMaster.h"

using namespace omnetpp;
using namespace std;

class Chain;
class ChainHash;

const string traceFileName = "results/poa_files/Monaco_0829_0830_20secs_Telecom.poa";

const int  NO_LOG=0, BASIC_LOG=1, DETAILED_LOG=2, VERY_DETAILED_LOG = 3; // levels of log to be written to the log file.

//costs and cpu demands of chains for different types of networks
const int Monaco=0, Lux=1, Uniform=2;
const vector <Cost_t> RT_ChainCostAtLvl    [] = {{544, 278, 164}, 							{100, 10   }, {68, 40 ,31}, 		 {68, 40, 31}};
const vector <Cost_t> Non_RT_ChainCostAtLvl[] = {{544, 278, 148, 86, 58, 47}, 	{100, 10, 1}, {68, 40, 29}, 		 {68, 40, 29}};
const vector <Cpu_t>  RT_ChainMu_u 				 [] = {{17, 17, 19}, 									{1, 	1 	 }, {17, 17, 19}, 		 {17, 17, 19}};
const vector <Cpu_t>  Non_RT_ChainMu_u 		 [] = {{17, 17, 17, 17, 17, 17},			{1, 	1, 	1}, {17, 17, 17}, 		 {17, 17, 17}};
const vector <Cpu_t>  nonAugmentedCpuAtLvl [] = {{11842, 111684, 112526, 113368, 114210, 115052}, {94, 188, 282, 376, 470, 564}, {1,	  1,	1}, {516, 1032, 1548}, {30, 60, 90}}; //$$ Monaco: 842, 1684, 2526, 3368, 4210, 5052
const int uniformChainMisgCost = 600;

// Parameters determining how to define whether a new generated chain is RT or not
const float RT_chain_pr = 0.5; // prob' that a new chain is an RT chain
const bool randomlySetChainType = false;
const bool evenChainsAreRt			= false;

// Mode: SYNC, or ASYNC
const bool mode=SYNC;

// Defines whether to print to the log and to the .res file the results of the BU stage of BUPU
const bool printBuRes = false; 

// Determining the level of debug and log.
const int  DEBUG_LVL=1, LOG_LVL=BASIC_LOG, RES_LVL=1;

										 
class MyConfig { 

		// A buffer for print-outs
		static const int 	bufSize = 128;
		static char 			buf[bufSize];
    static ofstream 	logFile, ResFile;

	public:

  	static unsigned int netType;
/*    static string 				traceFileName, */
    static string LogFileName, ResFileName;

		//Init
		static void openFiles ();
		
		//print
		static void printToRes (char* buf); 
		static void printToRes (string str);

		static void printToLog (char* buf); 
		static void printToLog (string str);
		static void printToLog (int d); // print "d,", where d is the integer, to the log file
		static void printToLog (vector <Chain> vec); // print all the IDs of the chains in the vec.
		static void printToLog (vector <ChainId_t> vec); // print a vec of ChainId_t to the log file
		static void printToLog (unordered_set <ChainId_t> set2print);
		static void printToLog (list <Chain> list2print, bool printLvl=true);
		static void printToLog (unordered_set <Chain, ChainHash> set2print);
		static void printToLog (Chain chain);
		static void printSuToLog (Chain chain);
		
		// Other accessories funcs'
		static ofstream getLogFile ();
		static vector<Cost_t> scalarProdcut (const vector<Cpu_t> &vec1, const vector<Cost_t> &vec2); // returns the scalar product of two vectors

		// erase the given key from the given set. Returns true iff the requested key was indeed found in the set
		static bool eraseKeyFromSet (unordered_set <ChainId_t> &set, ChainId_t id); 
};

#endif


