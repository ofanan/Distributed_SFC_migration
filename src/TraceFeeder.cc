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
    float  RT_chain_pr = 1.0; // prob' that a new chain is an RT chain
    int    RT_chain_rand_int = (int) (RT_chain_pr * (float) (RAND_MAX)); // the maximum randomized integer, for which we'll consider a new chain as a RT chain.
    unordered_map <int32_t, Chain> allChains; // chains are mapped by their IDs. 
    unordered_map <int16_t, unordered_set<int32_t> > chainsThatLeftDatacenter;//chainsThatLeftDC[i] will hold the list of IDs of chains that left DC i (either towards another PoA, or left the sim').
    unordered_map <int16_t, vector<Chain>> chainsThatJoinedPoa; // chainsThatJoinedPoa[i] will hold the list of chains that joined leaf (PoA) i
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
    void printAllChains (bool printPoa, bool printCurDatacenter); // print the list of all chains
		
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
	
	vector <int16_t> S_u = {8};
	Chain c0 = Chain (0, S_u);
	Chain c1 (1, S_u);
	unordered_set <Chain, ChainHash> dummy;
	dummy.insert (c0);
	
	// Now, after stage 0 is done, we know that the network and all the datacenters have woken up.
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

// Open input, output, and log files 
void TraceFeeder::openFiles () {
  outFile.open ("example.txt");
  outFile << networkName << endl;
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

void TraceFeeder::runTrace () {
	traceFile = ifstream (traceFileName);
	outFile   = ofstream (outFileName);
	
  string line;
  if (!traceFile.is_open ()) {
  	outFile << "Poa file was not found -> finishing simulation."; 
		endSimulation();
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
  		outFile << t << endl;
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
  outFile.close ();
}

// Print all the chains. Default: print only the chains IDs. 
void TraceFeeder::printAllChains (bool printPoa = false, bool printCurDatacenter = false)
{
	outFile << "allChains=";
	for (auto const & x : allChains) {
		outFile << x.first << " ";
	}	
	outFile << endl;
}

  	
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
The function inserts all the IDs of chains that left some datacenter dc to chainsThatLeftDatacenter[dc].
Inputs:
- line: a string, containing a list of the IDs of the chains that left the simulated area.
*/
void TraceFeeder::readChainsThatLeftLine (string line)
{
  char_separator<char> sep(" ");
  tokenizer<char_separator<char>> tokens(line, sep);
  Chain chain; // will hold the new chain to be inserted each time
  int32_t chain_id;
  
  // parse each old chain in the trace (.poa file), and find its current datacenter
  for (const auto& token : tokens) {
  	chain_id = stoi (token);
	  auto search = allChains.find(chain_id);
	  if (search == allChains.end()) {
			outFile << "Error in t=" << t << ": didn't find chain id " << chain_id << " that left\n";
			endSimulation();
	  }
	  else {
  		chainsThatLeftDatacenter[search->second.curDatacenter].insert (chain_id); // insert the id of the moved chain to the list of chains that left the current datacenter, where the chain is placed.
  		outFile << "erasing chain " << chain_id << endl;
  		allChains.erase (chain_id);// remove the chain from the list of chains.
	  }
  }
  outFile << "After reading chains that left: ";
  printAllChains ();
}

/*
Read a trace line that includes data about new chains.
Generate a new chain, and add it to the allChains.
Also, add the new generated chain to chainsThatJoinedPoa[poa], where poa is the curent poa of this new chain (poa is indicated in the trace, .poa file).
Inputs: 
- line: the line to parse. The line contains data in the format (c_1, poa_1)(c_2, poa_2), ... where poa_i is the updated poa of chain c_i.
*/
void TraceFeeder::readNewChainsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  int32_t chain_id;
  int16_t poa; 
	Chain chain; // will hold the new chain to be inserted each time
  
  
	for (const auto& token : tokens) {
		parseChainPoaToken (token, chain_id, poa);
		if (rand () < RT_chain_rand_int) {
			// Generate an RT (highest-priority) chain, and insert it to the beginning of the vector of chains that joined the relevant PoA (leaf DC)
			chain = RT_Chain     (chain_id, vector<int16_t> (pathToRoot[poa].begin(), pathToRoot[poa].begin()+RT_Chain::mu_u_len-1));
			chainsThatJoinedPoa[poa].insert (chainsThatJoinedPoa[poa].begin(), chain); // As this is an RT (highest-priority) chain, insert it to the beginning of the vector
		}
		else {
			// Generate a non-RT (lowest-priority) chain, and insert it to the end of the vector of chains that joined the relevant PoA (leaf DC)
			chain = Non_RT_Chain (chain_id, vector<int16_t> (pathToRoot[poa].begin(), pathToRoot[poa].begin()+Non_RT_Chain::mu_u_len-1)); 
			chainsThatJoinedPoa[poa].push_back (chain); // As this is a Non-RT (lowest-priority) chain, insert it to the end of the vector
		}
		allChains[chain_id] = chain; 
	}	
  outFile << "After readNewCHainsLine: ";
  printAllChains ();
}

/*
- Read a trace line that includes data about old chains, that moved and thus became critical.
- Find the chain in the db "allChains". 
- insert the chain to chainsThatJoinedPoa[poa], where poa is the new, updated poa.
- insert the chain to chainsThatLeftPoa[poa'], where poa' is the old, previous poa.
Inputs:
- line: the line to parse. The line contains data in the format (c_1, poa_1)(c_2, poa_2), ... where poa_i is the updated poa of chain c_i.
*/
// parse each old chain that became critical, and prepare data to be sent to its current place (to rlz its resources), and to its new PoA (to place that chain).
void TraceFeeder::readOldChainsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  int32_t chain_id;
  int16_t poa; 
	Chain chain; // will hold the new chain to be inserted each time

	for (const auto& token : tokens) {
		parseChainPoaToken (token, chain_id, poa);
	  auto search = allChains.find(chain_id);
	  if (search == allChains.end()) {
			outFile << "Error in t=" << t << ": didn't find chain id " << chain_id << " in allChains, in readChainsLine (old chains)\n";
//				endSimulation();
	  }
	  else {
			if (chain.isRT_Chain) {
				chainsThatJoinedPoa[poa].insert (chainsThatJoinedPoa[poa].begin(), chain); // As this is an RT (highest-priority) chain, insert it to the beginning of the vector
			}
			else {
				chainsThatJoinedPoa[poa].push_back (chain); // As this is a Non-RT (lowest-priority) chain, insert it to the end of the vector
			}
			chainsThatLeftDatacenter[chain.curDatacenter].insert (chain.id); // insert the id of the moved chain to the set of chains that left the current datacenter, where the chain is placed.
	  }
	}
  outFile << "After readOldCHainsLine: ";
  printAllChains ();
}

// Call each datacenters from which chains were moved (either to another datacenter, or merely left the sim').
void TraceFeeder::rlzRsrcsOfChains ()
{
	/*
	for (auto const& item : chainsThatLeftDatacenter)
	{
		outFile << "Chains that left dc " << item.first << ": ";
		for(auto chain_id : item.second) {
			outFile << chain_id << " ";			
		}    
		outFile << endl;
	}
	*/
}
  	
void TraceFeeder::initAlg () {  	
/*
	for (auto const& chain : chainsThatJoinedDatacenter)
	{
		outFile << "Chains that joined dc " << chain.first << ": ";
		for(auto chain_id : chain.second) {
			outFile << chain_id << " ";			
		}    
		outFile << endl;
	}
	*/
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

