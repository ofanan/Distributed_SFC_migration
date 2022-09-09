/************************************************************************************************************************************************
Controller of the simulation:
- reads the trace.
- runs ths placing algorithm, by calling the relevant datacenter.
- keeps tracks of the placing algorithm's results and costs.
**************************************************************************************************************************************************/
#ifndef SIM_CONTROLLER_H
#define SIM_CONTROLLER_H

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <regex>
#include <utility>
#include <cstdlib>
#include <chrono>

#include <vector>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>

#include "MyTypes.h"
#include "MyConfig.h"
#include "Datacenter.h"
#include "Chain.h"
#include "ChainsMaster.h"

using namespace omnetpp;
using namespace std;
using namespace boost;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

const DcId_t root_id = 0;

class Datacenter;

class SimController : public cSimpleModule
{
  private:
    cModule* network; // Pointer to the network on which the simulation is running
    string networkName; // name of the simulated netw: typically, either 'Lux', 'Monaco', or 'Tree'.
    DcId_t numDatacenters;
    DcId_t numLeaves;
    Lvl_t  height; // height of the tree
    bool isFirstPeriod = true; 
    bool isLastPeriod = false;
    int seed = 0; // default randomization value; may be overwritten when running a sim' campaign
    int numSeeds; // num of seeds to run a sim' for in a sim' camapaign
    int algStts; // stts of the last run of the alg'
    bool mode; // either Sync, or Async
    bool runningRtProbSim;
    int      RtChainRandInt; // the maximum randomized integer, for which we'll consider a new chain as a RT chain.
		string   RtSimResFileName;
    cMessage *curHandledMsg; // Incoming message that is currently handled.
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

		double RtProb; // prob' that a new chain is an RT chain

    //pathFromLeafToRoot[i][j] will hold the j-th hop in the path from leaf i to the root. E.g., pathFromLeafToRoot[i][0] will hold the dcId of leaf # i.
    vector <vector<DcId_t>> pathFromLeafToRoot; 
    //pathFromDcToRoot[i][j] will hold the j-th hop in the path from Dc i to the root. 
    vector <vector<DcId_t>> pathFromDcToRoot; 
    vector <vector <Lvl_t>> distTable; // dist[i][j] will hold the distance (in # of hosts) from DC i to DC i+j, where j>0

		int numMigsAtThisPeriod=0; // number of migration performed	at this period (according to the found sol).	
		int numBlockedUsrs;
		int numCritUsrs=0; // number of critical usrs at the beginning of this period
		float     period=1.0;
		Cpu_t lb, ub; // lower bound, upper bound amount of the cpu at the leaf for the binary search alg'
		bool lastBinSearchRun;
		float simLenInSec;
		float maxTraceTime; // when reading from the trace time above maxTraceTime, the trace run will finish
		
		string line; //current line being read from the tracefile
		
		//chainsThatLeftDc[i] will hold a vector of the (IDs of) chains that left DC i (either towards another leaf, or left the sim').
    unordered_map <DcId_t, vector<ChainId_t> > chainsThatLeftDc;
    unordered_map <DcId_t, vector<Chain>> chainsThatJoinedLeaf; // chainsThatJoinedLeaf[i] will hold the list of chains that joined the leaf with leafId i
    vector <Datacenter*> datacenters, leaves; // pointers to all the datacenters, and to all the leaves
		vector <ChainId_t> usrsThatLeft; // list of chains that left the simulated area in the cur time priod.
    
    //rcvdFinishedAlgMsgFromLeaves[i] will be true iff a message indicating the finish of the run of the sync placement alg' was rcvd from leaf i. Used only in Sync mode.
    vector <bool> rcvdFinishedAlgMsgFromLeaves; 
    

		// Init Functions
    void initialize(int stage);
    virtual int numInitStages() const {return 3;}; //2nd  stage: after all DCs are already initialized, discover shortest paths; 3rd stage: runTrace
		void setOutputFileNames ();
		void discoverPathsToRoot ();
		Lvl_t dist (DcId_t i, DcId_t j);
		void rst ();

		// Termination functions
		void finish ();

		// Other Functions
		void updateCpuAtLvl ();
		bool genRtChain (ChainId_t chainId); // returns true iff the next generated chain should be an RT chain. This is based on rand', and the prob' of RT.
		void runTrace  ();
		void runTimePeriod ();
		void readTraceLine ();
		void rdUsrsThatLeftLine (string line); // read a trace line, containing a list of chains that left the simulation
		void rdNewUsrsLine (string line); // read a trace line, containing a list of new chain and their updated PoAs.
		void rdOldUsrsLine (string line); // read a trace line, containing a list of old, moved chain and their updated PoAs.
		void rlzRsrcOfChains (unordered_map <DcId_t, vector<ChainId_t> > &ChainsToRlzFromDc); // Send a direct msg to each DC whose chains left, so that it releases its resources.
		void initAlg (); // init a placement alg'
  	void initAlgSync (); // init a sync placement alg'
  	void initAlgAsync (); // init an async placement alg'
  	inline void printBufToLog () {MyConfig::printToLog(buf);}
  	inline void printBufToRes () {MyConfig::printToRes(buf);}
		inline void updateRtChainRandInt (); // update the int (that is used to efficiently randomize whether a new cahin is RT or not), based on MyConfig::probOfRt.
		
    void handleMessage (cMessage *msg);
		void handlePlacementInfoMsg (cMessage *msg);
		void handleAlgMsg (cMessage *msg);
		void concludeTimePeriod (); // calc costs, move cur<--nxt in state variables, etc.
		void insertToChainsThatJoinedLeaf (DcId_t dcId, Chain chain);
		void insertToChainsThatLeftDc     (DcId_t dcId, ChainId_t chainId);//insert the given chainId to the vector of chains that left the given DC 
		void calcDistBetweenAllDcs (); // Calculate the distance (in num of hops) between each pair of datacenters.
		Lvl_t calcDistBetweenTwoDcs (DcId_t i, DcId_t); // Calculate the distance (in num of hops) between Dc i and Dc j
		inline Lvl_t idxInpathFromDcToRoot (DcId_t i, Lvl_t lvl);
		inline DcId_t leafId2DcId (DcId_t leafId);
		inline DcId_t dcId2leafId (DcId_t dcId);
		inline void genSettingsBuf (bool printTime=true);
    void openFiles ();
		void initBinSearchSim  (); // run a binary search for the minimum required rsrc aug' (actually, cpu at leaf) for finding a feasible sol'

		void continueBinSearch (); // After a failure / succesfully finishing the whole trace, this func' is called, to continue the binary search
    
    // Functions used for debugging
    void checkChainsMasterData (); // Compare the chainsManager's chains' location data to the datacenters' placedChains data.
    void checkDcsEndTimePeriod (); // Sanity check for the DCs when concluding a time period
    void printAllChainsPoas  	 (); //(ofstream &outFile, bool printSu, bool printleaf, bool printCurDatacenter); // print the PoA of each active user
    void PrintStateAndEndSim 	 (); // print the system's state, and end the simulation. 
		void printErrStrAndExit (const string &errorMsgStr);
				 
  public:
		int netType;
    ifstream traceFile;
    ofstream RtSimResFile; 
 		string LogFileName   = "example.txt";
 		string   RtSimFileName;
		static const int bufSize = 512;
		char buf[bufSize];
		static const int settingsBufSize = 256;
		char settingsBuf [settingsBufSize];
    SimController ();
    ~SimController ();
    void checkParams (); // Sanity checks for various parameters    
		void finishedAlg 		 (DcId_t dcId, DcId_t leafId);
		void preparePartialReshSync (DcId_t dcId, DcId_t leafId);
		void prepareFullReshSync ();
		void initFullReshSync ();
		void handleAlgFailure ();
		void printAllDatacenters (bool printPotPlaced=false, bool printPushUpList=false, bool printChainIds=true);
		void printBuCost  ();
		void genResLine   ();
		void printResLine ();
		void printResLine (ofstream outFile);
};

#endif
