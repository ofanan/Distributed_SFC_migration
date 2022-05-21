/*************************************************************************************************************************************************
Controller of the simulation:
- reads the trace.
- runs ths placing algorithm, by calling the relevant datacenter.
- keeps tracks of the placing algorithm's results and costs.
**************************************************************************************************************************************************/
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
	checkParams ();
	// Init the vectors of "datacenters", and the vector of "leaves", with ptrs to all DCs, and all leaves, resp.
	rcvdFinishedAlgMsgFromLeaves.resize(numLeaves);
	rcvdFinishedAlgMsgFromLeaves = {false};
	leaves.resize (numLeaves);
	datacenters.resize (numDatacenters);
	uint16_t leafId = 0;
	for (int dc(0); dc<numDatacenters; dc++) {
	  datacenters[dc] = (Datacenter*) network->getSubmodule("datacenters", dc);
	  if (bool(datacenters[dc]->par("isLeaf"))==1) {
	    leaves[leafId] = datacenters[dc];
	    leafId++;
	  }
	}
	discoverPathsToRoot ();
	runTrace ();	  
}

void SimController::checkParams ()
{

	for (uint16_t lvl(0); lvl < RT_Chain::cpuCostAtLvl.size()-1; lvl++) {
		if ((int)(RT_Chain::cpuCostAtLvl[lvl]) <= (int)(RT_Chain::cpuCostAtLvl[lvl+1])) {
			error ("RT_Chain::cpuCostAtLvl[] should be decreasing. However, RT_Chain::cpuCostAtLvl[%d]=%d, RT_Chain::cpuCostAtLvl[%d]=%d\n", 
							lvl, RT_Chain::cpuCostAtLvl[lvl], lvl+1, RT_Chain::cpuCostAtLvl[lvl+1]);
		}
	}
	for (uint16_t lvl(0); lvl < RT_Chain::cpuCostAtLvl.size()-1; lvl++) {
		if ((int)(Non_RT_Chain::cpuCostAtLvl[lvl]) <= (int)(Non_RT_Chain::cpuCostAtLvl[lvl+1])) {
			error ("Non_RT_Chain::cpuCostAtLvl[] should be decreasing. However, Non_RT_Chain::cpuCostAtLvl[%d]=%d, Non_RT_Chain::cpuCostAtLvl[%d]=%d\n", 
							lvl, Non_RT_Chain::cpuCostAtLvl[lvl], lvl+1, Non_RT_Chain::cpuCostAtLvl[lvl+1]);
		}
	}
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

/*************************************************************************************************************************************************
Run a single time step. Such a time step is supposed to include (at most) a single occurence of:
- A "t = " line.
- A "usr_that_left" line.
- A "new_usrs" line
- An "old_usrs" line.
**************************************************************************************************************************************************/
void SimController::runTimeStep () 
{
	isLastPeriod = true; // will reset this flag only if there's still new info to read from the trace
	if (!isFirstPeriod) {
	  concludeTimeStep (); // gather and print the results of the alg' in the previous time step
	}
	
  string line;
  while (getline (traceFile, line)) { 
  	if (line.compare("")==0 || (line.substr(0,2)).compare("//")==0 ){ // discard empty and comment lines
  	}
  	else if ( (line.substr(0,4)).compare("t = ")==0) {
  	
  		isLastPeriod = false;
  		// extract the t (time) from the traceFile, and update this->t accordingly.
  		char lineAsCharArray[line.length()+1];
  		strcpy (lineAsCharArray, line.c_str());
  		strtok (lineAsCharArray, " = ");
  		t = atoi (strtok (NULL, " = "));

			if (MyConfig::LOG_LVL>0) {
				snprintf (buf, bufSize, "t=%d\n", t);
				MyConfig::printToLog (buf); 
			}
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
  		// Schedule a self-event for reading the handling the next time-step
  		scheduleAt (simTime() + 1.0, new cMessage);
  	}
  }
}


void SimController::runTrace () {
	traceFile = ifstream (traceFileName);
	isFirstPeriod = true;
	
  numMigs         = 0; // will cnt the # of migrations in the current run
  if (!traceFile.is_open ()) {
  	error (".poa file was not found -> finishing simulation"); 
  }
	runTimeStep ();
}

void SimController::finish () 
{
  traceFile.close ();
//  concludeTimeStep (); 
	if (MyConfig::LOG_LVL>0) {
  	MyConfig::printToLog ("finished sim\n");
  }
}

/*************************************************************************************************************************************************
- Inc. numMigs for every chain where curDC!=nxtDc.
- Set for every chain curDc = nxtDc; nxtDc = UNPLACED.
- If running in sync mode: calculate and print the total cost
**************************************************************************************************************************************************/
void SimController::concludeTimeStep ()
{
	if (MyConfig::DEBUG_LVL>0) {
		for (auto const &chain : allChains) {
			if (chain.curLvl==UNPLACED_) {
				error ("t=$d: chain %d is unplaced at the end of cycle\n", t, chain.id);
			}
		}
	}
//	uint16_t numMigsSinceLastStep = 0;
	chainsThatJoinedLeaf.    clear ();
	chainsThatLeftDatacenter.clear ();
	rcvdFinishedAlgMsgFromLeaves = {false};
	
	if (MyConfig::LOG_LVL > 1) {
		printAllDatacenters ();
	}
}

void SimController::printAllDatacenters ()
{
	for (const auto datacenter : datacenters) {
		datacenter -> print ();
	}
}

// Returns the overall cpu cost at its current location.
int SimController::calcSolCpuCost () 
{
	
	int cpuCost = 0;
	for (auto const &chain : allChains) {	
		cpuCost += chain.getCpuCost ();
	}
	return cpuCost;
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


/*************************************************************************************************************************************************
Read and handle a trace line that details the IDs of chains that left the simulated area.
The function inserts all the IDs of chains that left some datacenter dc to chainsThatLeftDatacenter[dc].
Inputs:
- line: a string, containing a list of the IDs of the chains that left the simulated area.
**************************************************************************************************************************************************/
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
	  	if (chain.curLvl == UNPLACED_) {
				MyConfig::printToLog ("Note: this chain was not placed before leaving\n"); 
	  		continue;
	  	}
  		chainsThatLeftDatacenter[chain.getCurDatacenter()].push_back (chainId);  //insert the id of the moved chain to the vector of chains that left the current datacenter, where the chain is placed.
	  }
  }
}

/*************************************************************************************************************************************************
Read a trace line that includes data about new chains.
Generate a new chain, and add it to the allChains.
Also, add the new generated chain to chainsThatJoinedLeaf[leaf], where leaf is the curent leaf, co-located with the poa of this new chain (poa is indicated in the trace, .poa file).
Inputs: 
- line: the line to parse. The line contains data in the format (c_1, leaf_1)(c_2, leaf_2), ... where leaf_i is the updated PoA of chain c_i.
**************************************************************************************************************************************************/
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
			chain = RT_Chain (chainId, vector<uint16_t> {pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+RT_Chain::mu_u_len}); 
		}
		else {
			// Generate a non-RT (lowest-priority) chain, and insert it to the end of the vector of chains that joined the relevant leaf (leaf DC)
			chain = Non_RT_Chain (chainId, vector<uint16_t> (pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+Non_RT_Chain::mu_u_len)); 
		}
		insertSorted (chainsThatJoinedLeaf[poaId], chain); // insert the chain to its correct order in the (ordered) vector of chainsThatJoinedLeaf[poaId].
		allChains.insert (chain); 
	}	
	if (LOG_LVL>1) {
	  MyConfig::printToLog ("After readNewCHainsLine: ");
	  printAllChains (logFile);
	}
}

/*************************************************************************************************************************************************
- Read a trace line that includes data about old chains, that moved and thus became critical.
- Find the chain in the db "allChains". 
- insert the chain to chainsThatJoinedLeaf[leaf], where leaf is the new, updated leaf.
Inputs:
- line: the line to parse. The line contains data in the format (c_1, leaf_1)(c_2, leaf_2), ... where leaf_i is the updated leaf of chain c_i.
**************************************************************************************************************************************************/
void SimController::readOldChainsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  uint32_t chainId;
  uint16_t poaId;
	Chain chain; // will hold the chain found in this->allChains

	for (const auto& token : tokens) {
		parseChainPoaToken (token, chainId, poaId);
  	chainId = stoi (token);
  	if (!(findChainInSet (allChains, chainId, chain))) {
			error ("t=%d: didn't find chain id %d in allChains, in readOldChainsLine", t, chainId);
	  }
	  else {
			vector <uint16_t> S_u (pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+chain.mu_u_len ());
			Chain modifiedChain (chain.id, S_u); // will hold the modified chain to be inserted each time
			allChains.erase (chain); // remove the chain from our DB; will soon re-write it to the DB, having updated fields
			allChains.insert (modifiedChain);
			insertSorted (chainsThatJoinedLeaf[poaId], modifiedChain);			
			if (chain.curLvl != UNPLACED_) {
				// insert the id of the moved chain to the set of chains that left the current datacenter, where the chain is placed.
				chainsThatLeftDatacenter[chain.getCurDatacenter ()].push_back (modifiedChain.id); 
			}
	  }
	}
	
	if (LOG_LVL==2) {
	  logFile << "After readOldCHainsLine: ";
  	printAllChains (logFile);
  }
}


/*************************************************************************************************************************************************
- Call each datacenters from which chains were moved (either to another datacenter, or merely left the sim'), based on chainsThatLeftDatacenter.
- Clear chainsThatLeftDatacenter.
**************************************************************************************************************************************************/
void SimController::rlzRsrcsOfChains ()
{

	LeftChainsMsg* msg;
	uint16_t i;
	for (auto &item : chainsThatLeftDatacenter)
	{
		msg = new LeftChainsMsg ();
		msg -> setLeftChainsArraySize (item.second.size());
		i = 0;
		for (auto & chainId : item.second) {
			msg -> setLeftChains (i++, chainId);
		}
		sendDirect (msg, (cModule*)(datacenters[item.first]), "directMsgsPort");
	}
	chainsThatLeftDatacenter.clear ();
}

// Initiate the run of placement alg'
void SimController::initAlg () {  	

	return (MyConfig::mode==SYNC)? initAlgSync() : initAlgAsync();
}


// Initiate the run of an Sync placement alg'
void SimController::initAlgSync () 
{  	
	InitBottomUpMsg* msg;
	uint16_t i;
	bool *sentInitBottomUpMsg { new bool[numLeaves]{} }; // sentInitBottomUpMsg will be true iff we already sent a InitBottomUpMsg to leaf i
	
	// First, send InitBottomUpMsg to all the leaves to which new chains have joined.
	for (auto const& item : chainsThatJoinedLeaf)
	{
		if (LOG_LVL==2) {
			logFile << "Chains that joined dc " << item.first << ": ";
		}
		
		msg = new InitBottomUpMsg ();
		msg -> setNotAssignedArraySize (item.second.size());
		i = 0;
		for(auto &chain : item.second) {
			msg -> setNotAssigned (i++, chain);
		}    
		sendDirect (msg, (cModule*)(leaves[item.first]), "directMsgsPort");
		sentInitBottomUpMsg[item.first] = true;
	}

	// Next, send (empty) InitBottomUpMsg to the remainder leaves, just to initiate sync' BUPU.
	for (uint16_t leafId(0); leafId < numLeaves; leafId++) {
		if (!(sentInitBottomUpMsg[leafId])) {
			msg = new InitBottomUpMsg ();
			msg -> setNotAssignedArraySize (0);
			sendDirect (msg, (cModule*)(leaves[leafId]), "directMsgsPort");
		}
	}	
	delete[] sentInitBottomUpMsg;
}

// Initiate the run of a Async placement alg'
void SimController::initAlgAsync () {  	

	InitBottomUpMsg* msg;
	uint16_t i;
	for (auto const& item : chainsThatJoinedLeaf)
	{
		if (LOG_LVL==2) {
			logFile << "Chains that joined dc " << item.first << ": ";
		}
		
		msg = new InitBottomUpMsg ();
		msg -> setNotAssignedArraySize (item.second.size());
		i = 0;
		for(auto &chain : item.second) {
			msg -> setNotAssigned (i++, chain);
		}    
		sendDirect (msg, (cModule*)(leaves[item.first]), "directMsgsPort");
	}
}

void SimController::handlePlacementInfoMsg (cMessage *msg)
{
	PlacementInfoMsg* curHandledMsg = (PlacementInfoMsg*) (msg);
	Chain chain;
	int16_t  curLvl;
	uint32_t chainId;

	for (uint16_t i(0); i< (uint16_t) (curHandledMsg -> getNewlyPlacedChainsIdsArraySize()); i++) {
		
		curLvl = ((Datacenter*)curHandledMsg->getSenderModule())->lvl; 
		chainId 			= curHandledMsg -> getNewlyPlacedChainsIds (i);
		if (!(findChainInSet (allChains, chainId, chain))) {
			error ("t=%d: didn't find chain id %d that appeared in a PlacementInfoMsg", t, chainId);

		}
		else {
			if (chain.getCurDatacenter()!=UNPLACED) { // was it an old chain that migrated?
				numMigs++; // Yep --> inc. the mig. cntr.
			}
			allChains.erase (chain); // remove the chain from our DB; will soon re-write it to the DB, having updated fields
			chain.curLvl = curLvl;
			allChains.insert (chain);
		}
	}
}

void SimController::handleFinishedAlgMsg (cMessage *msg)
{
	uint16_t leafId = ((Datacenter*) (msg->getSenderModule()) )->leafId;
	rcvdFinishedAlgMsgFromLeaves [leafId] = true; 
	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "rcvd fin alg msg from DC %d leaf %d\n", ((Datacenter*) (msg->getSenderModule()) )->id, leafId);
		MyConfig::printToLog (buf);
	}
	
	bool rcvdFinishedAlgMsgFromAllLeaves = true;
	for (uint16_t i(0); i < numLeaves; i++) {
		if (!rcvdFinishedAlgMsgFromLeaves[i]) {
			rcvdFinishedAlgMsgFromAllLeaves = false;
		}
	}
	if (rcvdFinishedAlgMsgFromAllLeaves) {
		if (MyConfig::LOG_LVL>=DETAILED_LOG) {
			MyConfig::printToLog ("rcvd fin alg msg from all leaves ******************\n");
		}
	}
}

void SimController::handleAskReshSyncMsg (cMessage *msg)
{
	vector <Chain> ChainsToMigrate = findChainsByPoa (allChains, ((Datacenter*) (msg->getSenderModule()) )->id);
}

void SimController::handleMessage (cMessage *msg)
{
  if (msg -> isSelfMessage()) {
		isFirstPeriod = false;
		if (!isLastPeriod) {
			runTimeStep ();
		}
  }
  else if (dynamic_cast<PlacementInfoMsg*> (msg)) { 
  	handlePlacementInfoMsg (msg);
  }
  else if (dynamic_cast<FinishedAlgMsg*> (msg)) { 
  	handleFinishedAlgMsg (msg);
  }
  else if (dynamic_cast<AskReshSyncMsg*> (msg)) { 
		handleAskReshSyncMsg (msg);
  }
  else if (dynamic_cast<PrintAllDatacentersMsg*> (msg)) { 
  	MyConfig::printToLog ("rcvd PrintAllDatacentersMsg\n");
  	printAllDatacenters ();
  	endSimulation (); //$$$
  }
  else {
  	error ("Rcvd unknown msg type");
  }
  delete (msg);
}

