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

#include "Datacenter.h"
#include "Chain.h"
#include "Parameters.h"
#include "initBottomUpMsg_m.h"

using namespace omnetpp;
using namespace std;
using namespace boost;

const int16_t root_id = 0;
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
    float  RT_chain_pr = 0.5; // prob' that a new chain is an RT chain
    int    RT_chain_rand_int = (int) (RT_chain_pr * (float) (RAND_MAX)); // the maximum randomized integer, for which we'll consider a new chain as a RT chain.
    int16_t numNewChains, numCritChains;
    set <Chain> allChains, newChains, critChains;
    set <int> curInts;
    map <int16_t, set<int32_t>> chainsThatLeft; // chainsThatLeft[i] will hold the list of chains that left user i (either moved to another PoA, or left the sim').
    map <int16_t, set<Chain>> critAndMovedChainsOfLeaf; // will hold the critical chain of each leaf (poa) that currently has critical chains.
    vector <Datacenter*> datacenters; // pointes to all the datacenters
    vector <Datacenter*> leaves;      // pointes to all the leaves
    vector <vector<int16_t>> pathToRoot; //pathToRoot[i][j] will hold the j-th hop in the path from leaf i to the root. In particular, pathToRoot[i][0] will hold the datacenter id of leaf # i.
    void initialize(int stage);
    virtual int numInitStages() const {return 2;}; //Use 2nd init stage, after all DCs are already initialized, for discovering the path from each leaf to the root.

		// Functions
		void openFiles ();
		void discoverPathsToRoot ();

		void runTrace  ();
		void readChainsThatLeftLine (string line); // read a trace line, containing a list of chains that left the simulation
		void readChainsLine (string line, bool isNewChainsLine); // read a trace line, containing a list of chain and their updated PoAs.
		void initAlg ();
    void handleMessage (cMessage *msg);
		
  public:
    string traceFileName = "results/poa_files/Tree_short.poa";
 		string outFileName   = "example.txt";
    ofstream outFile;
    ifstream traceFile;
    TraceFeeder ();
    ~TraceFeeder ();
    void parseChainPoaToken (string token, int32_t &chain_id, int16_t &poa);
};

Define_Module(TraceFeeder);

TraceFeeder::TraceFeeder() {
}

TraceFeeder::~TraceFeeder() {}

void TraceFeeder::initialize (int stage)
{
  if (stage==0) {
		network         = (cModule*) (getParentModule ()); // No "new", because then need to dispose it.
		networkName 		= (network -> par ("name")).stdstringValue();
		numDatacenters  = (int16_t) (network -> par ("numDatacenters"));
		numLeaves       = (int16_t) (network -> par ("numLeaves"));
		height       		= (int16_t) (network -> par ("height"));
		srand(seed); // set the seed of random num generation
		return;
	}
	
	openFiles ();
	// Init the vectors of "datacenters", and the vector of "leaves", with ptrs to all DCs, and all leaves, resp.
	leaves.resize (numLeaves);
	datacenters.resize (numDatacenters);
	int leaf_id = 0;
	for (int dc(0); dc<numDatacenters; dc++) {
	  datacenters[dc] = (Datacenter*) network->getSubmodule("datacenters", dc);
	  if (bool(datacenters[dc]->par("isLeaf"))==1) {
	    leaves[leaf_id++] = datacenters[dc];
	  }
	}
	discoverPathsToRoot ();
	runTrace ();	  
}

void TraceFeeder::discoverPathsToRoot () {
	pathToRoot.resize (numLeaves);
	int16_t dc_id = 0;
	for (int16_t leaf_id(0) ; leaf_id < numLeaves; leaf_id++)  {
		pathToRoot[leaf_id].resize (height);
		dc_id = leaves[leaf_id]->id;
	  int height = 0;
		while (dc_id != root_id) {
		 	dc_id = datacenters[dc_id]->idOfParent;
		 	pathToRoot[leaf_id][height] = dc_id;
		}
	}
}

// Open input, output, and log files 
void TraceFeeder::openFiles () {
  outFile.open ("example.txt");
  outFile << networkName << endl;
}

// Open input, output, and log files 
void TraceFeeder::runTrace () {
	traceFile = ifstream (traceFileName);
	outFile   = ofstream (outFileName);
	
  string line;
  if (!traceFile.is_open ()) {
  	outFile << "wrong poa file name -> finishing simulation."; 
		endSimulation();
  }
  while (getline (traceFile, line)) { 
  	if (line.compare("")==0 || (line.substr(0,2)).compare("//")==0 ){ // discard empty and comment lines
  	}
  	else if ( (line.substr(0,4)).compare("t = ")==0) {
  		char lineAsCharArray[line.length()+1];
  		strcpy (lineAsCharArray, line.c_str());
  		strtok (lineAsCharArray, " = ");
  		t = atoi (strtok (NULL, " = "));
  		outFile << t << endl;
  	}
  	else if ( (line.substr(0,14)).compare("usrs_that_left")==0) {
  		readChainsThatLeftLine (line.substr(15));
  	} 	
  	else if ( (line.substr(0,8)).compare("new_usrs")==0) {
  		readChainsLine (line.substr(9), true); 
  	}
  	else if ( (line.substr(0,8)).compare("old_usrs")==0) {
  		readChainsLine (line.substr(9), false);
  		
  		// Now, that we finished reading and parsing all the data about new / old critical chains, call a placement algorithm to place all the new / critical chains.
  		initAlg ();
  	}
  }
  traceFile.close ();
  outFile.close ();
}
  	
  	
void TraceFeeder::initAlg () {  	
//		critAndMovedChainsOfLeaf[poa].insert (newChain); // insert the new chain to the set of crit' chains of the relevant leaf.
//		critAndMovedChainsOfLeaf[poa].insert (critChain); // insert the moved chain to the set of crit'/moved chains of its new PoA=leaf.
//		critAndMovedChainsOfLeaf[curDatacenter].insert (critChain); // insert the moved chain to the set of crit'/moved chains of its new PoA=leaf.	
}  	
//  	char_separator<char> slashSlash("//");
//	  tokenizer<char_separator<char>> tokens(text, sep);
//    for (const auto& t : tokens) {
//        outFile << t << "." << endl;
//    }

//  	outFile << line << endl;
//stringstream ss("bla bla");
//string s;

//while (getline(ss, s, ' ')) {
// cout << s << endl;
//}


// parse a token of the type "u,poa" where u is the chain_id number and poa is the user's current Poa
void TraceFeeder::parseChainPoaToken (string token, int32_t &chain_id, int16_t &poa)
{
	istringstream newChainToken(token); 
  string numStr; 
	getline (newChainToken, numStr, ',');
	chain_id = stoi (numStr);
	getline (newChainToken, numStr, ',');
	poa = stoi (numStr);
	if (poa > numLeaves) {
		outFile << "Error at t=" << t << ": poa is " << poa << "while the number of leaves in the network is only " << numLeaves << "; EXITING" << endl;
		endSimulation();
	}
}


/*
Read and handle a trace line that details the IDs of chains that left the simulated area.
Inputs:
- line: a string, containing a list of the IDs of the chains that left the simulated area.
*/
void TraceFeeder::readChainsThatLeftLine (string line)
{
  char_separator<char> sep(" ");
  tokenizer<char_separator<char>> tokens(line, sep);
  int32_t chain_id;
  
  // parse each old chain in the trace (.poa file), find its delay feasible datacenters, and insert it into the set of new chains
  outFile <<  "chains that left: ";
  for (const auto& token : tokens) {
  	outFile << token << ",";
  	//chainsThatLeft[chain.curDatacenter].insert (chain.id); // insert the id of the moved chain to the list of chains that left the current datacenter, where the chain is placed.
  }
  outFile << endl;
}

/*
Read a trace line that includes data about new / critical chains.
Inputs:
- line: the line to parse. The line contains data in the format (c_1, poa_1)(c_2, poa_2), ... where poa_i is the updated poa of chain c_i.
- isNewChainsLine: when true, the line details new chains.
If the chain is new, the function adds it to the set of new chains.
Else, the chain finds the chain in the db "all Chains", and inserts the chain to the set of critical chains.
*/
void TraceFeeder::readChainsLine (string line, bool isNewChainsLine)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  int32_t chain_id;
  int16_t poa; 
  Chain chain; // will hold the new chain to be inserted each time
  
  // parse each old chain in the trace (.poa file), find its delay feasible datacenters, and insert it into the set of new chains
  for (const auto& token : tokens) {
  	parseChainPoaToken (token, chain_id, poa);
  	if (isNewChainsLine) {
			if (rand () < RT_chain_rand_int) { // randomly decided that this is an RT chain 
				chain = Non_RT_Chain (chain_id, vector<int16_t> (pathToRoot[poa].begin(), pathToRoot[poa].begin()+RT_Chain::mu_u_len-1)); 
			}
			else {
				chain = Non_RT_Chain (chain_id, vector<int16_t> (pathToRoot[poa].begin(), pathToRoot[poa].begin()+Non_RT_Chain::mu_u_len-1)); 
			}
			newChains.insert (chain); 
		}
  	else {
			findChainInSet (allChains, chain_id, chain); // find the moved chain.
			critChains.insert (chain); // insert the moved chain to the list of critical chains.
			chainsThatLeft[chain.curDatacenter].insert (chain.id); // insert the id of the moved chain to the list of chains that left the current datacenter, where the chain is placed.
  	}
  }
}


void TraceFeeder::handleMessage (cMessage *msg)
{
  if (msg -> isSelfMessage()) {
    outFile << "rcvd self msg\n"; 
  Chain foundChain;
  int32_t id = 0;
  outFile << "nxtDC=" << foundChain.nxtDatacenter << " \n";
  }

    // If self.msg:
    // - Read from the trace the data for a single slot.
    // - Init the placement alg'
    // - Schedule an event for the next run
}


//  EV << "sent direct msg\n";
//  outFile << "Writing this to a file.\n";
//  outFile.close();

    // Discover the input gates of all the datacenters.
//    directMsgsPort
//    for (int dc(0); dc < numDatacenters; dc++) {
//      gateOfDatacenters[dc] = 
//        cGate *outGate    = gate("port$o", portNum);
//    }
//sendDirect(cMessage *msg, cModule *mod, int gateId)
//sendDirect(cMessage *msg, cModule *mod, const char *gateName, int index=-1)
//sendDirect(cMessage *msg, cGate *gate)
//cModule *targetModule = getParentModule()->getSubmodule("node2");
//sendDirect(new cMessage("msg"), targetModule, "in");
//directMsgsPort
    // Open the trace input file
    // Schedule a self-event for beginning reading the trace

