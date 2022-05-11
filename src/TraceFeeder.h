#ifndef TRACE_FEEDER_H
#define TRACE_FEEDER_H

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

using namespace omnetpp;
using namespace std;
using namespace boost;

const int16_t root_id = 0;
const int8_t LOG_LVL = 1;

class TraceFeeder : public cSimpleModule
{
  private:
    cModule* network; // Pointer to the network on which the simulation is running
    string networkName; // name of the simulated netw: typically, either 'Lux', 'Monaco', or 'Tree'.
    int16_t numDatacenters;
    int16_t numLeaves;
    int16_t height; // height of the tree
    int     t; //sim time (in seconds)
    uint32_t seed = 42;
    float  RT_chain_pr = 1.0; // prob' that a new chain is an RT chain
    int    RT_chain_rand_int = (int) (RT_chain_pr * (float) (RAND_MAX)); // the maximum randomized integer, for which we'll consider a new chain as a RT chain.
    unordered_set <Chain, ChainHash> allChains; // All the currently active chains. 

		uint32_t numMigs=0; // number of migration performed		
		
		//chainsThatLeftDC[i] will hold a vector of the (IDs of) chains that left DC i (either towards another leaf, or left the sim').
    unordered_map <int16_t, vector<int32_t> > chainsThatLeftDatacenter;
    unordered_map <int16_t, vector<Chain>> chainsThatJoinedLeaf; // chainsThatJoinedLeaf[i] will hold the list of chains that joined leaf i
    vector <Datacenter*> datacenters, leaves; // pointers to all the datacenters, and to all the leaves
    vector <vector<int16_t>> pathToRoot; //pathToRoot[i][j] will hold the j-th hop in the path from leaf i to the root. In particular, pathToRoot[i][0] will hold the datacenter id of leaf # i.

		// Init Functions
    void initialize(int stage);
    virtual int numInitStages() const {return 2;}; //Use 2nd init stage, after all DCs are already initialized, for discovering the path from each leaf to the root.
		void openFiles ();
		void discoverPathsToRoot ();

		// Other Functions
		void runTrace  ();
		void readChainsThatLeftLine (string line); // read a trace line, containing a list of chains that left the simulation
		void readNewChainsLine (string line); // read a trace line, containing a list of new chain and their updated PoAs.
		void readOldChainsLine (string line); // read a trace line, containing a list of old, moved chain and their updated PoAs.
		void rlzRsrcsOfChains (); // Send a direct msg to each DC whose chains left, so that it releases its resources.
		void initAlg (); // init a placement alg'
  	void initAlgSync (); // init a sync placement alg'
  	void InitAlgAsync (); // init an async placement alg'
		
    void handleMessage (cMessage *msg);
		void concludeTimeStep (); // calc costs, move cur<--nxt in state variables, etc.
		int calcSolCpuCost (); // returns the overall CPU cost
    
    // Functions used for debugging
		void printChain (ofstream &outFile, const Chain &chain, bool printSu);
    void printAllChains (ofstream &outFile, bool printSu, bool printleaf, bool printCurDatacenter); // print the list of all chains
				 
  public:
    string traceFileName = "results/poa_files/Tree_short.poa";
    ifstream traceFile;
 		string LogFileName   = "example.txt";
    ofstream logFile;
    TraceFeeder ();
    ~TraceFeeder ();
    void parseChainPoaToken (string token, int32_t &chainId, int16_t &poaId);
};

#endif
