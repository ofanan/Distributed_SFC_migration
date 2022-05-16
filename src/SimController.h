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

#include <vector>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>

#include "Datacenter.h"
#include "Chain.h"
#include "initBottomUpMsg_m.h"
#include "leftChainsMsg_m.h"
#include "placementInfoMsg_m.h"
#include "finishedAlgMsg_m.h"

using namespace omnetpp;
using namespace std;
using namespace boost;

const int16_t root_id = 0;
const int8_t LOG_LVL = 1;

class SimController : public cSimpleModule
{
  private:
    cModule* network; // Pointer to the network on which the simulation is running
    string networkName; // name of the simulated netw: typically, either 'Lux', 'Monaco', or 'Tree'.
    uint16_t numDatacenters;
    uint16_t numLeaves;
    uint8_t  height; // height of the tree
    uint32_t t; //sim time (in seconds)
    bool isFirstPeriod = true; 
    bool isLastPeriod = false;
    uint32_t seed = 42;
    float  	 RT_chain_pr = 1.0; // prob' that a new chain is an RT chain
    int      RT_chain_rand_int = (int) (RT_chain_pr * (float) (RAND_MAX)); // the maximum randomized integer, for which we'll consider a new chain as a RT chain.
    unordered_set <Chain, ChainHash> allChains; // All the currently active chains. 
    cMessage *curHandledMsg; // Incoming message that is currently handled.

		uint32_t numMigs=0; // number of migration performed		
		
		//chainsThatLeftDC[i] will hold a vector of the (IDs of) chains that left DC i (either towards another leaf, or left the sim').
    unordered_map <uint16_t, vector<int32_t> > chainsThatLeftDatacenter;
    unordered_map <uint16_t, vector<Chain>> chainsThatJoinedLeaf; // chainsThatJoinedLeaf[i] will hold the list of chains that joined leaf i
    vector <Datacenter*> datacenters, leaves; // pointers to all the datacenters, and to all the leaves
    vector <bool> rcvdFinishedAlgMsgFromLeaves; //rcvdFinishedAlgMsgFromLeaves[i] will be true iff a message indicating the finish of the run of the sync placement alg' was rcvd from leaf i
    vector <vector<uint16_t>> pathToRoot; //pathToRoot[i][j] will hold the j-th hop in the path from leaf i to the root. In particular, pathToRoot[i][0] will hold the datacenter id of leaf # i.

		// Init Functions
    void initialize(int stage);
    virtual int numInitStages() const {return 2;}; //Use 2nd init stage, after all DCs are already initialized, for discovering the path from each leaf to the root.
		void openFiles ();
		void discoverPathsToRoot ();

		// Termination functions
		void finish ();

		// Other Functions
		void runTrace  ();
		void runTimeStep ();
		void readChainsThatLeftLine (string line); // read a trace line, containing a list of chains that left the simulation
		void readNewChainsLine (string line); // read a trace line, containing a list of new chain and their updated PoAs.
		void readOldChainsLine (string line); // read a trace line, containing a list of old, moved chain and their updated PoAs.
		void rlzRsrcsOfChains (); // Send a direct msg to each DC whose chains left, so that it releases its resources.
		void initAlg (); // init a placement alg'
  	void initAlgSync (); // init a sync placement alg'
  	void initAlgAsync (); // init an async placement alg'
		
    void handleMessage (cMessage *msg);
		void handlePlacementInfoMsg (cMessage *msg);
		void handleFinishedAlgMsg (cMessage *msg);
		void concludeTimeStep (); // calc costs, move cur<--nxt in state variables, etc.
		int calcSolCpuCost (); // returns the overall CPU cost
    
    // Functions used for debugging
		void printChain (ofstream &outFile, const Chain &chain, bool printSu);
		void printAllDatacenters ();
    void printAllChains (ofstream &outFile, bool printSu, bool printleaf, bool printCurDatacenter); // print the list of all chains
				 
  public:
    string traceFileName = "results/poa_files/Tree_shorter.poa";
    ifstream traceFile;
 		string LogFileName   = "example.txt";
		static const uint16_t bufSize = 128;
		char buf[bufSize];
		ofstream logFile;
    SimController ();
    ~SimController ();
    void parseChainPoaToken (string const token, uint32_t &chainId, uint16_t &poaId);
    void checkParams (); // Sanity checks for various parameters
};

#endif
