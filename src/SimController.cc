#include "SimController.h"

Define_Module(SimController);

SimController::SimController() {
}

SimController::~SimController() {}

void SimController::initialize (int stage)
{

  if (stage==0) {
		network         = (cModule*) (getParentModule ()); // No "new", because then need to dispose it.
		networkName 		= (network -> par ("name")).stdstringValue();
		numDatacenters  = (uint16_t) (network -> par ("numDatacenters"));
		numLeaves       = (uint16_t) (network -> par ("numLeaves"));
		height       		= (uint8_t) (network -> par ("height"));
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
void SimController::openFiles () {
	MyConfig::openFiles ();
}

// Fill this->pathToRoot.
// pathToRoot[i] will hold the path from leaf i to the root.
void SimController::discoverPathsToRoot () {
	pathToRoot.resize (numLeaves);
	uint16_t dc_id;
	for (uint16_t leafId(0) ; leafId < numLeaves; leafId++)  {
		pathToRoot[leafId].resize (height);
		dc_id = leaves[leafId]->id;
	  int height = 0;
		while (dc_id != root_id) {
		 	pathToRoot[leafId][height++] = dc_id;
		 	dc_id = datacenters[dc_id]->idOfParent;
		}
	}
}

/*
Run a single time step. Such a time step is supposed to include (at most) a single occurence of:
- A "t = " line.
- A "usr_that_left" line.
- A "new_usrs" line
- An "old_usrs" line.
*/
void SimController::runTimeStep () 
{
  string line;
  while (getline (traceFile, line)) { 
  	if (line.compare("")==0 || (line.substr(0,2)).compare("//")==0 ){ // discard empty and comment lines
  	}
  	else if ( (line.substr(0,4)).compare("t = ")==0) {
  	
  		// extract the t (time) from the traceFile, and update this->t accordingly.
  		char lineAsCharArray[line.length()+1];
  		strcpy (lineAsCharArray, line.c_str());
  		strtok (lineAsCharArray, " = ");
  		t = atoi (strtok (NULL, " = "));

  		uint32_t bufSize = 128;
  		char buf[bufSize];
  		snprintf (buf, bufSize, "t%d=\n", t);
  		MyConfig::printToLog (buf); 
  	}
  	else if ( (line.substr(0,14)).compare("usrs_that_left")==0) {
  		readChainsThatLeftLine (line.substr(15));
  	} 	
  	else if ( (line.substr(0,8)).compare("new_usrs")==0) {
  		readNewChainsLine (line.substr(9)); 
  	}
  	else if ( (line.substr(0,8)).compare("old_usrs")==0) {
  		MyConfig::printToLog ("In old usrs\n");
  		readOldChainsLine (line.substr(9));
  		
  		// Now, that we finished reading and parsing all the data about new / old critical chains, rlz the rsrcs of chains that left their current location, and then call a placement algorithm to 
  		// place all the new / critical chains.
  		rlzRsrcsOfChains ();
//  		initAlg ();
  		concludeTimeStep ();
  		// Schedule a self-event for reading the handling the next time-step
  		scheduleAt (simTime() + 1.0, new cMessage);
  	}
  }
}


void SimController::runTrace () {
	traceFile = ifstream (traceFileName);
	
  numMigs         = 0; // will cnt the # of migrations in the current run
  if (!traceFile.is_open ()) {
  	error (".poa file was not found -> finishing simulation"); 
  }
	runTimeStep ();
}

void SimController::finish () 
{
  traceFile.close ();
}

/*
- Inc. numMigs for every chain where curDC!=nxtDc.
- Set for every chain curDc = nxtDc; nxtDc = UNPLACED.
- If running in sync mode: calculate and print the total cost
*/
void SimController::concludeTimeStep ()
{
//	uint16_t numMigsSinceLastStep = 0;
//	numMigs += numMigsSinceLastStep;
	chainsThatJoinedLeaf.    clear ();
	chainsThatLeftDatacenter.clear ();
}

// Return the overall cpu cost at the NEXT cycle (based on the chain.curDatacenter).
int SimController::calcSolCpuCost () 
{
	int cpuCost = 0;
	for (auto const chain : allChains) {
		cpuCost += (chain.isRT_Chain)? RT_Chain::cpuCostAtLvl[datacenters[chain.curDatacenter]->lvl] : Non_RT_Chain::cpuCostAtLvl[datacenters[chain.curDatacenter]->lvl];
	}
	return cpuCost;
}

// Print a data about a single chain to a requested output file.
void SimController::printChain (ofstream &outFile, const Chain &chain, bool printSu=true)
{
	outFile << "chain " << chain.id;
	if (printSu) {
		outFile << " S_u=";
		for (auto it (chain.S_u.begin()); it <= chain.S_u.end(); it++){
			outFile << *it << ";";
		}
	}
	outFile << endl;
}



// Print all the chains. Default: print only the chains IDs. 
void SimController::printAllChains (ofstream &outFile, bool printSu=false, bool printleaf=false, bool printCurDatacenter=false)
{
	MyConfig::printToLog ("allChains\n*******************\n");
	for (auto const & chain : allChains) {
		outFile << "chain " << chain.id;
		if (printSu) {
			for (auto it (chain.S_u.begin()); it <= chain.S_u.end(); it++){
				outFile << *it << ";";
			}
		}
		outFile << endl;
	}	
	outFile << endl;
}

  	
// parse a token of the type "u,poa" where u is the chainId number and poas is the user's current poa
void SimController::parseChainPoaToken (string const token, uint32_t &chainId, uint16_t &poaId)
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
void SimController::readChainsThatLeftLine (string line)
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
				MyConfig::printToLog ("Note: this chain was not placed before leaving\n"); 
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
void SimController::readNewChainsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  uint32_t chainId;
  uint16_t poaId; 
	Chain chain; // will hold the new chain to be inserted each time
  
	for (const auto& token : tokens) {
		parseChainPoaToken (token, chainId, poaId);
		if (rand () < RT_chain_rand_int) {
			chain = RT_Chain (chainId, vector<uint16_t> {pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+RT_Chain::mu_u_len-1}); 
		}
		else {
			// Generate a non-RT (lowest-priority) chain, and insert it to the end of the vector of chains that joined the relevant leaf (leaf DC)
			chain = Non_RT_Chain (chainId, vector<uint16_t> (pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+Non_RT_Chain::mu_u_len)); 
		}
		insertSorted (chainsThatJoinedLeaf[poaId], chain); // insert the chain to its correct order in the (ordered) vector of chainsThatJoinedLeaf[poaId].
		allChains.insert (chain); 
	}	
	if (LOG_LVL==2) {
	  MyConfig::printToLog ("After readNewCHainsLine: ");
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
void SimController::readOldChainsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  uint32_t chainId;
  uint16_t poaId;
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
void SimController::rlzRsrcsOfChains ()
{

	leftChainsMsg* msg;
	uint16_t i;
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
void SimController::initAlg () {  	

	return (MyConfig::mode==SYNC)? initAlgSync() : initAlgAsync();
}


// Initiate the run of an Sync placement alg'
void SimController::initAlgSync () 
{  	
	initBottomUpMsg* msg;
	uint16_t i;
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

// Initiate the run of a Async placement alg'
void SimController::initAlgAsync () {  	

	initBottomUpMsg* msg;
	uint16_t i;
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

void SimController::handleMessage (cMessage *msg)
{
  if (msg -> isSelfMessage()) {
  	MyConfig::printToLog ("rcvd self msg\n");
  	runTimeStep ();
  }
  else if (dynamic_cast<placementInfoMsg*> (msg)) { 
  	placementInfoMsg* msg2handle = (placementInfoMsg*) (msg);
  	Chain chain;
  	int16_t  datacenterId;
  	uint32_t chainId;

  	for (uint16_t i(0); i< (uint16_t) (msg2handle -> getNewlyPlacedChainsArraySize()); i++) {
  		
  		datacenterId = ((Datacenter*)msg->getSenderModule())->id; 
  		chainId 			= msg2handle -> getNewlyPlacedChains (i);
			if (!(findChainInSet (allChains, chainId, chain))) {
				error ("t=%d: didn't find chain id %d that appeared in a placementInfoMsg", t, chainId);

			}
			else {
				if (chain.curDatacenter!=UNPLACED) { // was it an old chain that migrated?
					numMigs++; // Yep --> inc. the mig. cntr.
				}
				allChains.erase (chain); // remove the chain from our DB; will soon re-write it to the DB, having updated fields
				chain.curDatacenter = datacenterId;
				allChains.insert (chain);
			}
  	}
  	delete (msg);
  }
  else {
  	error ("Rcvd unknown msg type");
  }
}
