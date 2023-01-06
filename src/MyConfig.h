// Configuration parameters, writing to log files, and accessory functions.

#ifndef MY_CONFIG_H
#define MY_CONFIG_H
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
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
struct ChainHash;

const string tracePath = "res/poa_files/";
const int  NO_LOG=0, BASIC_LOG=1, DETAILED_LOG=2, VERY_DETAILED_LOG = 3, TLAT_DETAILED_LOG=4; // levels of log to be written to the log file.

//costs and cpu demands of chains for different types of networks
const int MonacoIdx=0, LuxIdx=1, UniformTreeIdx=2, NonUniformTreeIdx=3; // Possible NetType values
const int uniformChainMigCost = 600;

// Mode: Sync, or Async
const bool Sync  = 0;
const bool Async = 1;

class MyConfig { 

		// A buffer for print-outs
		static const int 	bufSize = 512;
		static const int  cityNameLen = 10;
		static char       cityName[cityNameLen];
		static char 			buf[bufSize];
    static ofstream 	logFile, resFile, comOhResFile;

		// directions 0, 1, ..., lvlOfRoot-1 indicate pkts where the source is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt); 
		// from lvlOfRoot there're no northbound packet. 
		// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts where the source is 1,2, ... lvlOfRoot, and the direction is south (to child).
		static vector <uint32_t> pktCnt; // pktCnt[i] will hold the # of pkts sent in direction i
		static vector <uint64_t> bitCnt; // bitCnt[i] will hold the # of bits sent in direction i

	public:

		// simulation mode: currently, either Sync/Async. This variable may be changed along the sim. In particular, for large (Lux/Monaco) sim, 
		// we begin the sim' in Sync mode, for easy init; switch to mode==Async at the second decision period
		static int mode; 
		
		const  static int modeStrLen=20;
		static char modeStr[modeStrLen]; // a string characterizing the mode of operation ("syncPartResh", "SyncFullResh", "Async"). 
		static string traceFileName;
		static int netType;
		static int  overallNumBlockedUsrs; 
		static Lvl_t height;// height of the tree 
		static Lvl_t lvlOfRoot; // level of the root (typically height-1).
		static const vector <Cpu_t> nonAugmentedCpuAtLeaf; 
		static const vector <vector <Cost_t>> RtChainCostAtLvl;
		static const vector <vector <Cost_t>> NonRtChainCostAtLvl;
		static const vector <vector <Cpu_t>>  RtChainMu_u;
		static const vector <vector <Cpu_t>>  NonRtChainMu_u;
		static vector <Cpu_t> minCpuToPlaceAnyChainAtLvl;
		static Cpu_t  cpuAtLeaf;
		static vector <Cpu_t> cpuAtLvl; 
		static Lvl_t lvlOfHighestReshDc; //lvl of the highest that reshuffled at the cur period; UNPLACED_LVL if there was no resh at this period
		static bool useFullResh; // when true, a failure cause a global reshuffle, of all chains and datacenters
		static bool discardAllMsgs; // when true, all DCs ignore all incoming msgs.
		static float FModePeriod; // period of a Dc staying in F Mode after the last reshuffle msg arrives
    static float traceTime; //current time (in the trace, in seconds)
    static bool runningBinSearchSim; 
    static bool runningRtProbSim;
    static bool runningCampaign;
    static bool measureRunTime;
		static bool manuallySetPktSize;
		static bool allowBlkChain;
		static bool isFirstPeriod; // will be true iff this the first decision period in the sim
		static short int bitLenOfDcId, bitLenOfChainId, bitLenOfClassId, bitLenOfCpuAlloc, bitLenOfDefCpu;
		static short int  bitLenOfRtChain, bitLenOfNonRtChain, bitLenOfHdr, bitLenOfRtChainInPdReqPkt, bitLenOfNonRtChainInPdReqPkt, basicBitLenOfPdPkt;
	  static vector <float> BU_ACCUM_DELAY_OF_LVL;
	  static vector <float> RESH_ACCUM_DELAY_OF_LVL;

		// Parameters determining how to define whether a new generated chain is RT or not
		static bool randomlySetChainType;
		static bool evenChainsAreRt;

		// parameters determining the debug and output files (.log and .res files)
    static string logFileName, resFileName, comOhResFileName;
		static int 	LOG_LVL;
		static int 	DEBUG_LVL;
		static int 	RES_LVL;
		static bool logDelays; // when true, the sim generates a log of the delays
		static bool logComOh; // when true, the sim generates a log of the comm overhead (num and sizes of pkts).
		static bool printBuRes; // when true, print to the log and to the .res file the results of the BU stage of BUPU
		static bool printBupuRes; // when true, print to the log the results of BUPU
		static bool notifiedReshInThisPeriod; // true iff already notified about resh in this period in the log file

		//Init and reset
		static void init ();
		static void rst ();
		static bool openFiles ();
		
		//print
		static void printToRes (char* buf); 
		static void printToRes (string str);
    static void printToLog (vector <vector <int32_t>> mat);
		static void printToLog (char* buf); 
		static void printToLog (char* buf, ofstream outFile);
		static void printToLog (string str);
		static void printToLog (const int d); // print "d,", where d is the integer, to the log file
		static void printToLog (vector <Chain> vec); // print all the IDs of the chains in the vec.
		static void printToLog (vector <ChainId_t> vec); // print a vec of ChainId_t to the log file
		/*		static void printToLog (vector <DcId_t> vec);*/
		static void printToLog (unordered_set <ChainId_t> set2print);
		static void printToLog (list <Chain> list2print, bool printLvl=true);
		static void printToLog (unordered_set <Chain, ChainHash> set2print);
		static void printToLog (Chain chain);
		static void printSuToLog (Chain chain);
		
		// Other accessories funcs'
    static inline bool fileExists (const std::string& name);
		static string exec(const char* cmd);
		static void setNetTypeFromString (string str);
		static int  getNetTypeFromString (string str);
		static int 	getOverallNumPkts ();
		static ofstream getlogFile ();
		static vector<Cost_t> scalarProdcut (const vector<Cpu_t> &vec1, const vector<Cost_t> &vec2); // returns the scalar product of two vectors
		static bool incCntr (Lvl_t cntrNum, int64_t bitCnt);

		// erase the given key from the given set. Returns true iff the requested key was indeed found in the set
		static bool eraseKeyFromSet (unordered_set <ChainId_t> &set, ChainId_t id); 
		friend class SimController;
};

#endif

