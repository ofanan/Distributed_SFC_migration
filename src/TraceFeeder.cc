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

#include "MyConfig.h"
#include "Datacenter.h"
#include "Chain.h"
#include "Parameters.h"
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
		void rlzRsrcsOfChains ();
		void initAlg ();
    void handleMessage (cMessage *msg);
    
    // Functions used for debugging
		void printChain (ofstream &outFile, const Chain &chain);
    void printAllChains (ofstream &outFile, bool printSu, bool printleaf, bool printCurDatacenter); // print the list of all chains
				 
  public:
    string traceFileName = "results/poa_files/Tree_short.poa";
 		string LogFileName   = "example.txt";
    ofstream logFile;
    ifstream traceFile;
    TraceFeeder ();
    ~TraceFeeder ();
    void parseChainPoaToken (string token, int32_t &chainId, int16_t &poaId);
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
	
	// Now, after stage 0 is done, we know that the network and all the datacenters have woken up.
	openFiles ();
	// Init the vectors of "datacenters", and the vector of "leaves", with ptrs to all DCs, and all leaves, resp.
	leaves.resize (numLeaves);
	datacenters.resize (numDatacenters);
	int leafId = 0;
	for (int dc(0); dc<numDatacenters; dc++) {
	  datacenters[dc] = (Datacenter*) network->getSubmodule("datacenters", dc);
	  if (bool(datacenters[dc]->par("isLeaf"))==1) {
	    leaves[leafId++] = datacenters[dc];
	  }
	}
	discoverPathsToRoot ();
	runTrace ();	  
}

// Open input, output, and log files 
void TraceFeeder::openFiles () {
  logFile.open ("example.txt");
  logFile << networkName << endl;
}


void TraceFeeder::discoverPathsToRoot () {
	pathToRoot.resize (numLeaves);
	int16_t dc_id;
	for (int16_t leafId(0) ; leafId < numLeaves; leafId++)  {
		pathToRoot[leafId].resize (height);
		dc_id = leaves[leafId]->id;
	  int height = 0;
		while (dc_id != root_id) {
		 	pathToRoot[leafId][height++] = dc_id;
		 	dc_id = datacenters[dc_id]->idOfParent;
		}
	}
}

void TraceFeeder::runTrace () {
	traceFile = ifstream (traceFileName);
	
  string line;
  if (!traceFile.is_open ()) {
  	error (".poa file was not found -> finishing simulation"); 
  }
  while (getline (traceFile, line)) { 
  	if (line.compare("")==0 || (line.substr(0,2)).compare("//")==0 ){ // discard empty and comment lines
  	}
  	else if ( (line.substr(0,4)).compare("t = ")==0) {
  	
  		// extract the t (time) from the traceFile, and update this->t accordingly.
  		char lineAsCharArray[line.length()+1];
  		strcpy (lineAsCharArray, line.c_str());
  		strtok (lineAsCharArray, " = ");
  		t = atoi (strtok (NULL, " = "));
  		logFile << "t=" << t << endl;
  	}
  	else if ( (line.substr(0,14)).compare("usrs_that_left")==0) {
  		readChainsThatLeftLine (line.substr(15));
  	} 	
  	else if ( (line.substr(0,8)).compare("new_usrs")==0) {
  		readNewChainsLine (line.substr(9)); 
  	}
  	else if ( (line.substr(0,8)).compare("old_usrs")==0) {
  		readOldChainsLine (line.substr(9));
  		
  		// Now, that we finished reading and parsing all the data about new / old critical chains, rlz the rsrcs of chains that left their current location, and then call a placement algorithm to 
  		// place all the new / critical chains.
  		rlzRsrcsOfChains ();
  		initAlg ();
  	}
  }
  traceFile.close ();
  logFile.close ();
}


// Print a data about a single chain to a requested output file.
void TraceFeeder::printChain (ofstream &outFile, const Chain &chain)
{
	outFile << "chain " << chain.id << ": S_u=";
	for (vector <int16_t>::const_iterator it (chain.S_u.begin()); it <= chain.S_u.end(); it++){
		outFile << *it << ";";
	}
	outFile << endl;
}



// Print all the chains. Default: print only the chains IDs. 
void TraceFeeder::printAllChains (ofstream &outFile, bool printSu=false, bool printleaf=false, bool printCurDatacenter=false)
{
	outFile << "allChains\n*******************\n";
	for (auto const & chain : allChains) {
		outFile << "chain " << chain.id << ": ";
		for (vector <int16_t>::const_iterator it (chain.S_u.begin()); it <= chain.S_u.end(); it++){
			outFile << *it << ";";
		}
//		for_each(chain.S_u.begin(), chain.S_u.end(),[](int number){cout << number << ";";});
		outFile << endl;
	}	
	outFile << endl;
}

  	
// parse a token of the type "u,poa" where u is the chainId number and poas is the user's current poa
void TraceFeeder::parseChainPoaToken (string token, int32_t &chainId, int16_t &poaId)
{
	istringstream newChainToken(token); 
  string numStr; 
	getline (newChainToken, numStr, ',');
	chainId = stoi (numStr);
	getline (newChainToken, numStr, ',');
	poaId = stoi (numStr);
	if (poaId > numLeaves) {
		error ("t=%d. : .poa file includes poa ID %d while the number of leaves in the network is only %d", t, poaId, numLeaves);
	}
}


/*
Read and handle a trace line that details the IDs of chains that left the simulated area.
The function inserts all the IDs of chains that left some datacenter dc to chainsThatLeftDatacenter[dc].
Inputs:
- line: a string, containing a list of the IDs of the chains that left the simulated area.
*/
void TraceFeeder::readChainsThatLeftLine (string line)
{
  char_separator<char> sep(" ");
  tokenizer<char_separator<char>> tokens(line, sep);
  Chain chain; // will hold the new chain to be inserted each time
  int32_t chainId;
  
  // parse each old chain in the trace (.poa file), and find its current datacenter
  for (const auto& token : tokens) {
  	chainId = stoi (token);
  	if (!(findChainInSet (allChains, chainId, chain))) {
			error ("t=%d: didn't find chain id %d that left", t, chainId);
	  }
	  else {
	  	if (chain.curDatacenter == notPlacedYet) {
	  		logFile << "Note: this chain was not placed before leaving\n";
	  		continue;
	  	}
  		chainsThatLeftDatacenter[chain.curDatacenter].push_back (chainId);  //insert the id of the moved chain to the vector of chains that left the current datacenter, where the chain is placed.
	  }
  }
}

/*
Read a trace line that includes data about new chains.
Generate a new chain, and add it to the allChains.
Also, add the new generated chain to chainsThatJoinedLeaf[leaf], where leaf is the curent leaf, co-located with the poa of this new chain (poa is indicated in the trace, .poa file).
Inputs: 
- line: the line to parse. The line contains data in the format (c_1, leaf_1)(c_2, leaf_2), ... where leaf_i is the updated PoA of chain c_i.
*/
void TraceFeeder::readNewChainsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  int32_t chainId;
  int16_t poaId; 
	Chain chain; // will hold the new chain to be inserted each time
  
	for (const auto& token : tokens) {
		parseChainPoaToken (token, chainId, poaId);
		if (rand () < RT_chain_rand_int) {
			chain = RT_Chain (chainId, vector<int16_t> {pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+RT_Chain::mu_u_len-1}); 
			chainsThatJoinedLeaf[poaId].insert (chainsThatJoinedLeaf[poaId].begin(), chain); // As this is an RT (highest-priority) chain, insert it to the beginning of the vector
		}
		else {
			// Generate a non-RT (lowest-priority) chain, and insert it to the end of the vector of chains that joined the relevant leaf (leaf DC)
			chain = Non_RT_Chain (chainId, vector<int16_t> (pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+Non_RT_Chain::mu_u_len-1)); 
			chainsThatJoinedLeaf[poaId].push_back (chain); // As this is a Non-RT (lowest-priority) chain, insert it to the end of the vector
		}
		allChains.insert (chain);
	}	
	if (LOG_LVL==2) {
	  logFile << "After readNewCHainsLine: ";
	  printAllChains (logFile);
	}
}

/*
- Read a trace line that includes data about old chains, that moved and thus became critical.
- Find the chain in the db "allChains". 
- insert the chain to chainsThatJoinedLeaf[leaf], where leaf is the new, updated leaf.
Inputs:
- line: the line to parse. The line contains data in the format (c_1, leaf_1)(c_2, leaf_2), ... where leaf_i is the updated leaf of chain c_i.
*/
// parse each old chain that became critical, and prepare data to be sent to its current place (to rlz its resources), and to its new leaf (to place that chain).
void TraceFeeder::readOldChainsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  int32_t chainId;
  int16_t poaId;
	Chain chain; // will hold the new chain to be inserted each time

	for (const auto& token : tokens) {
		parseChainPoaToken (token, chainId, poaId);
  	chainId = stoi (token);
  	if (!(findChainInSet (allChains, chainId, chain))) {
			error ("t=%d: didn't find chain id %d in allChains, in readOldChainsLine", t, chainId);
	  }
	  else {
			
			allChains.erase (chain); // remove the chain from our DB; will soon re-write it to the DB, having updated fields
			chain.S_u = pathToRoot[poaId]; //Update S_u of the chain to reflect its new location
			allChains.insert (chain);
			if (chain.isRT_Chain) {
				chainsThatJoinedLeaf[poaId].insert (chainsThatJoinedLeaf[poaId].begin(), chain); // As this is an RT (highest-priority) chain, insert it to the beginning of the vector
			}
			else {
				chainsThatJoinedLeaf[poaId].push_back (chain); // As this is a Non-RT (lowest-priority) chain, insert it to the end of the vector
			}
			
			if (chain.curDatacenter != notPlacedYet) {
				chainsThatLeftDatacenter[chain.curDatacenter].push_back (chain.id); // insert the id of the moved chain to the set of chains that left the current datacenter, where the chain is placed.
			}
	  }
	}
	
	if (LOG_LVL==2) {
	  logFile << "After readOldCHainsLine: ";
  	printAllChains (logFile);
  }
}

// Call each datacenters from which chains were moved (either to another datacenter, or merely left the sim').
void TraceFeeder::rlzRsrcsOfChains ()
{

	leftChainsMsg* msg;
	int16_t i;
	for (auto &item : chainsThatLeftDatacenter)
	{
		msg = new leftChainsMsg ();
		msg -> setLeftChainsArraySize (item.second.size());
		i = 0;
		for (auto & chainId : item.second) {
			msg -> setLeftChains (i++, chainId);
		}
		sendDirect (msg, (cModule*)(datacenters[item.first]), "directMsgsPort");
	}
}


void TraceFeeder::initAlg () {  	

	initBottomUpMsg* msg;
	int16_t i;
	for (auto const& item : chainsThatJoinedLeaf)
	{
		if (LOG_LVL==2) {
			logFile << "Chains that joined dc " << item.first << ": ";
		}
		
		msg = new initBottomUpMsg ();
		msg -> setNotAssignedArraySize (item.second.size());
		i = 0;
		for(auto &chain : item.second) {
			msg -> setNotAssigned (i++, chain);
		}    
		sendDirect (msg, (cModule*)(datacenters[item.first]), "directMsgsPort");
		endSimulation ();
	}
}


void TraceFeeder::handleMessage (cMessage *msg)
{
  if (msg -> isSelfMessage()) {
  }
  else if (dynamic_cast<placementInfoMsg*> (msg)) { 
  	placementInfoMsg* msg2handle = (placementInfoMsg*) (msg);
  	Chain chain;
  	int16_t datacenterId;
  	int32_t chainId;

  	for (uint16_t i(0); i< (uint16_t) (msg2handle -> getPlacedChainsArraySize()); i++) {
  		
  		datacenterId 	= msg2handle -> getDatacenterId ();
  		chainId 			= msg2handle -> getPlacedChains (i);
			if (!(findChainInSet (allChains, chainId, chain))) {
				error ("t=%d: didn't find chain id %d that appeared in a placementInfoMsg", t, chainId);

			}
			else {
				allChains.erase (chain); // remove the chain from our DB; will soon re-write it to the DB, having updated fields
				chain.nxtDatacenter = datacenterId;
				allChains.insert (chain);
			}
  	}
  }
}

//$$$
// 4.9.2: raising errors:
// error ("rgrgrg", %d);

