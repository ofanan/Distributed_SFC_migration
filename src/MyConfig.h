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

const string tracePath = "res/poa_files/";
const int  NO_LOG=0, BASIC_LOG=1, DETAILED_LOG=2, VERY_DETAILED_LOG = 3; // levels of log to be written to the log file.

//costs and cpu demands of chains for different types of networks
const int MonacoIdx=0, LuxIdx=1, UniformTreeIdx=2, NonUniformTreeIdx=3; // Possible NetType values
const vector <Cost_t> Non_RtChainCostAtLvl [] = {{544, 278, 148, 86, 58, 47}, 	{544, 278, 148, 86, 58, 47}, {100, 10, 1}, {68, 40, 29}};
const vector <Cpu_t>  RtChainMu_u 				  [] = {{17, 17, 19}, 								{17, 17, 19},								 {1, 	1 	 },  {17, 17, 19}};
const vector <Cpu_t>  Non_RtChainMu_u 		  [] = {{17, 17, 17, 17, 17, 17},			{17, 17, 17, 17, 17, 17}, 	 {1, 	1, 	1},  {17, 17, 17}};
const int uniformChainMigCost = 600;

// Parameters determining how to define whether a new generated chain is RT or not
const float RtChain_pr = 0.3; // prob' that a new chain is an RT chain
const bool randomlySetChainType = false;
const bool evenChainsAreRt			= false;

// Mode: SYNC, or ASYNC
const bool SYNC  = true;
const bool ASYNC = false;
const bool mode=SYNC;

// Determining the level of debug and log.
const int  DEBUG_LVL=1, LOG_LVL=VERY_DETAILED_LOG, RES_LVL=1;

										 
class MyConfig { 

		// A buffer for print-outs
		static const int 	bufSize = 128;
		static char 			buf[bufSize];
    static ofstream 	logFile, resFile;

	public:

		static string traceFileName;
		static int netType;
		static char mode_str[12]; 
		static const vector <Cpu_t> nonAugmentedCpuAtLeaf; 
		static const vector <vector <Cost_t>> RtChainCostAtLvl;
		static const vector <vector <Cost_t>> Non_RtChainCostAtLvl;
		static const vector <vector <Cpu_t>>  RtChainMu_u;
		static const vector <vector <Cpu_t>>  Non_RtChainMu_u;
		static Cpu_t  cpuAtLeaf;
		static Lvl_t lvlOfHighestReshDc; //lvl of the highest that reshuffled at the cur period; UNPLACED_LVL if there was no resh at this period

		static const bool printBuRes; // when true, print to the log and to the .res file the results of the BU stage of BUPU
		static bool notifiedReshInThisPeriod; // true iff already notified about resh in this period in the log file

    static string logFileName, resFileName;

		//Init
		static void init ();
		static bool openFiles ();
		
		//print
		static void printToRes (char* buf); 
		static void printToRes (string str);
    static void printToLog (vector <vector <int32_t>> mat);
		static void printToLog (char* buf); 
		static void printToLog (string str);
		static void printToLog (const int d); // print "d,", where d is the integer, to the log file
		static void printToLog (vector <Chain> vec); // print all the IDs of the chains in the vec.
		static void printToLog (vector <ChainId_t> vec); // print a vec of ChainId_t to the log file
		static void printToLog (unordered_set <ChainId_t> set2print);
		static void printToLog (list <Chain> list2print, bool printLvl=true);
		static void printToLog (unordered_set <Chain, ChainHash> set2print);
		static void printToLog (Chain chain);
		static void printSuToLog (Chain chain);
		
		// Other accessories funcs'
		static void setNetTypeFromString (string str);
		static int  getNetTypeFromString (string str);
		static ofstream getlogFile ();
		static vector<Cost_t> scalarProdcut (const vector<Cpu_t> &vec1, const vector<Cost_t> &vec2); // returns the scalar product of two vectors

		// erase the given key from the given set. Returns true iff the requested key was indeed found in the set
		static bool eraseKeyFromSet (unordered_set <ChainId_t> &set, ChainId_t id); 
};

#endif


