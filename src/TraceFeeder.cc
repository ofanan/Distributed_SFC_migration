#include "TraceFeeder.h"

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
	MyConfig::openFiles ();
  MyConfig::printToLog ("rgrg");
}

// Fill this->pathToRoot.
// pathToRoot[i] will hold the path from leaf i to the root.
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
	
  numMigs         = 0; // will cnt the # of migrations in the current run
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
  		string str = "abcdefg";
//  		sprintf (str, "t=%d\n", t);
  		MyConfig::printToLog (str);
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
//  		concludeTimeStep ();
  	}
  }
  traceFile.close ();
  logFile.close ();
}

/*
- Inc. numMigs for every chain where curDC!=nxtDc.
- Set for every chain curDc = nxtDc; nxtDc = UNPLACED.
- If running in sync mode: calculate and print the total cost
*/
void TraceFeeder::concludeTimeStep ()
{
	int16_t numMigsSinceLastStep = 0;
	for (auto chain : allChains) {
		if (chain.nxtDatacenter != chain.curDatacenter) {
			numMigsSinceLastStep++;
			chain.curDatacenter = chain.nxtDatacenter;
		}
		chain.nxtDatacenter = UNPLACED;
	}
	numMigs += numMigsSinceLastStep;
}

// Return the overall cpu cost at the NEXT cycle (based on the chain.nxtDatacenter).
int TraceFeeder::calcSolCpuCost () 
{
	int cpuCost = 0;
	for (auto const chain : allChains) {
		cpuCost += (chain.isRT_Chain)? RT_Chain::cpuCostAtLvl[datacenters[chain.nxtDatacenter]->lvl] : Non_RT_Chain::cpuCostAtLvl[datacenters[chain.nxtDatacenter]->lvl];
	}
	return cpuCost;
}

// Print a data about a single chain to a requested output file.
void TraceFeeder::printChain (ofstream &outFile, const Chain &chain, bool printSu=true)
{
	outFile << "chain " << chain.id;
	if (printSu) {
		outFile << " S_u=";
		for (vector <int16_t>::const_iterator it (chain.S_u.begin()); it <= chain.S_u.end(); it++){
			outFile << *it << ";";
		}
	}
	outFile << endl;
}



// Print all the chains. Default: print only the chains IDs. 
void TraceFeeder::printAllChains (ofstream &outFile, bool printSu=false, bool printleaf=false, bool printCurDatacenter=false)
{
	outFile << "allChains\n*******************\n";
	for (auto const & chain : allChains) {
		outFile << "chain " << chain.id;
		if (printSu) {
			for (vector <int16_t>::const_iterator it (chain.S_u.begin()); it <= chain.S_u.end(); it++){
				outFile << *it << ";";
			}
		}
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
	  	if (chain.curDatacenter == UNPLACED) {
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
		}
		else {
			// Generate a non-RT (lowest-priority) chain, and insert it to the end of the vector of chains that joined the relevant leaf (leaf DC)
			chain = Non_RT_Chain (chainId, vector<int16_t> (pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+Non_RT_Chain::mu_u_len)); 
		}
		insertSorted (chainsThatJoinedLeaf[poaId], chain); // insert the chain to its correct order in the (ordered) vector of chainsThatJoinedLeaf[poaId].
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
			insertSorted (chainsThatJoinedLeaf[poaId], chain);			
			if (chain.curDatacenter != UNPLACED) {
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

// Initiate the run of placement alg'
void TraceFeeder::initAlg () {  	

	if (MyConfig::mode==SYNC) {
		return initAlgSync();
	}
	return InitAlgAsync();
}


// Initiate the run of a Sync placement alg'
void TraceFeeder::initAlgSync () {  	

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
		sendDirect (msg, (cModule*)(leaves[item.first]), "directMsgsPort");
	}
}

// Initiate the run of an Async placement alg'
void TraceFeeder::InitAlgAsync () 
{  	
	initBottomUpMsg* msg;
	int16_t i;
	bool *sentInitBottomUpMsg { new bool[numLeaves]{} }; // sentInitBottomUpMsg will be true iff we already sent a initBottomUpMsg to leaf i
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
		sendDirect (msg, (cModule*)(leaves[item.first]), "directMsgsPort");
		sentInitBottomUpMsg[item.first] = true;
	}

	for (uint16_t leafId(0); leafId < numLeaves; leafId++) {
		if (!(sentInitBottomUpMsg[leafId])) {
			msg = new initBottomUpMsg ();
			msg -> setNotAssignedArraySize (0);
			sendDirect (msg, (cModule*)(leaves[leafId]), "directMsgsPort");
		}
	}	
	delete[] sentInitBottomUpMsg;
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
  else {
  	error ("Rcvd unknown msg type");
  }
}

