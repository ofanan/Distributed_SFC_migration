/*************************************************************************************************************************************************
Controller of the simulation:
- reads the trace.
- runs ths placing algorithm, by calling the relevant datacenter.
- keeps tracks of the placing algorithm's results and costs.
**************************************************************************************************************************************************/
#include "SimController.h"

class Datacenter;

// returns true iff the given datacenter dcId, at the given level, is delay-feasible for this chain (namely, appears in its S_u)

Define_Module(SimController);

SimController::SimController() {
}

SimController::~SimController() {}

void SimController::initialize (int stage)
{

  if (stage==0) {
		network         = (cModule*) (getParentModule ()); // No "new", because then need to dispose it.
		networkName 		= (network -> par ("name")).stdstringValue();
		int netType 		= MyConfig::getNetTypeFromString (networkName);
		if (netType<0) {
			error ("The .ini fileName you chose implies a wrong netType");
		}
		MyConfig::netType=netType;
		numDatacenters  = (DcId_t) (network -> par ("numDatacenters"));
		numLeaves       = (DcId_t) (network -> par ("numLeaves"));
		height       		= (Lvl_t) (network -> par ("height"));
		srand(seed); // set the seed of random num generation
		return;
	}
	
	openFiles ();
	checkParams (); 
	// Init the vectors of "datacenters", and the vector of "leaves", with ptrs to all DCs, and all leaves, resp.
	rcvdFinishedAlgMsgFromLeaves.resize(numLeaves);
	fill(rcvdFinishedAlgMsgFromLeaves.begin(), rcvdFinishedAlgMsgFromLeaves.end(), false);
	leaves.resize (numLeaves);
	datacenters.resize (numDatacenters);
	DcId_t leafId = 0;
	for (int dc(0); dc<numDatacenters; dc++) {
	  datacenters[dc] = (Datacenter*) network->getSubmodule("datacenters", dc);
	  if (bool(datacenters[dc]->par("isLeaf"))==1) {
	    leaves[leafId] = datacenters[dc];
	    leaves[leafId]->setLeafId (leafId);
	    leafId++;
	  }
	}
	discoverPathsToRoot ();
	ChainsMaster::clear ();
	runTrace ();
}

void SimController::openFiles ()
{
	MyConfig::LogFileName = "example.txt";
	MyConfig::ResFileName = "res.res";
	
	int traceNetType = MyConfig::getNetTypeFromString (traceFileName);
	if (traceNetType!=MyConfig::netType) {	
//	results/poa_files/
		string errorMsgStr = "traceFileName is " + traceFileName;
    char errorMsg[errorMsgStr.length() + 1];
    strcpy(errorMsg, errorMsgStr.c_str());
		error (errorMsg);
		  // declaring character array
//		error ("traceFileName is " + traceFileName); //%s while .ini file is %s.ini", traceFileName, networkName);
//		error ("traceFileName (aka .poa file) doesn't correspond .ini fileName");
	}
	// Now, after stage 0 is done, we know that the network and all the datacenters have woken up.
	if (!MyConfig::openFiles ()) {
		error ("MyConfig::openFiles failed");
	}
}

void SimController::checkParams ()
{

	for (int lvl(0); lvl < RT_Chain::costAtLvl.size()-1; lvl++) {
		if ((int)(RT_Chain::costAtLvl[lvl]) <= (int)(RT_Chain::costAtLvl[lvl+1])) {
			error ("RT_Chain::costAtLvl[] should be decreasing. However, RT_Chain::costAtLvl[%d]=%d, RT_Chain::costAtLvl[%d]=%d\n", 
							lvl, RT_Chain::costAtLvl[lvl], lvl+1, RT_Chain::costAtLvl[lvl+1]);
		}
	}
	for (int lvl(0); lvl < RT_Chain::costAtLvl.size()-1; lvl++) {
		if ((int)(Non_RT_Chain::costAtLvl[lvl]) <= (int)(Non_RT_Chain::costAtLvl[lvl+1])) {
			error ("Non_RT_Chain::costAtLvl[] should be decreasing. However, Non_RT_Chain::costAtLvl[%d]=%d, Non_RT_Chain::costAtLvl[%d]=%d\n", 
							lvl, Non_RT_Chain::costAtLvl[lvl], lvl+1, Non_RT_Chain::costAtLvl[lvl+1]);
		}
	}
}

/*************************************************************************************************************************************************
Fill this->pathToRoot. pathToRoot[i] will hold the path from leaf i to the root.
**************************************************************************************************************************************************/
void SimController::discoverPathsToRoot () {
	pathToRoot.resize (numLeaves);
	DcId_t dcId;
	for (int leafId(0) ; leafId < numLeaves; leafId++)  {
		pathToRoot[leafId].resize (height);
		dcId = leaves[leafId]->dcId;
	  int height = 0;
		while (dcId != root_id) {
		 	pathToRoot[leafId][height++] = dcId;
		 	dcId = datacenters[dcId]->idOfParent;
		}
	}
}

/*************************************************************************************************************************************************
Run a single time step. Such a time step is assumed to include (at most) a single occurence of:
- A "t = " line.
- A "usr_that_left" line.
- A "new_usrs" line
- An "old_usrs" line. This should be the last line for the time step.
**************************************************************************************************************************************************/
void SimController::runTimePeriod () 
{
	isLastPeriod = true; // will reset this flag only if there's still new info to read from the trace
	if (!isFirstPeriod) {
	  concludeTimePeriod (); // gather and print the results of the alg' in the previous time step
	}
	
  string line;
  
  // discard empty and comment lines
  while (getline (traceFile, line)) { 
  	if (line.compare("")==0 || (line.substr(0,2)).compare("//")==0 ){ 
  	}

		if ( (line.substr(0,4)).compare("t = ")==0) {
			
			isLastPeriod = false;
			// extract the t (time) from the traceFile, and update this->t accordingly.
			char lineAsCharArray[line.length()+1];
			strcpy (lineAsCharArray, line.c_str());
			strtok (lineAsCharArray, " = ");
			int new_t = atoi (strtok (NULL, " = "));
			if (DEBUG_LVL>0 && new_t <= t) {
				error ("error in trace file: t is not incremented. t=%d, new_t=%d", t, new_t);
			}
			t = new_t;

			if (LOG_LVL>0) {
				snprintf (buf, bufSize, "\n\nt = %d\n********************", t);
				MyConfig::printToLog (buf); 
			}
		}
		else if ( (line.substr(0,14)).compare("usrs_that_left")==0) {
			rdUsrsThatLeftLine (line.substr(15));
		} 	
		else if ( (line.substr(0,8)).compare("new_usrs")==0) {
			rdNewUsrsLine (line.substr(9)); 
		}
		else if ( (line.substr(0,8)).compare("old_usrs")==0) {
			rdOldUsrsLine (line.substr(9));
			
			//Finished parsing the data about new and critical chains --> rlz rsrcs of chains that left their current location, and then call a placement algorithm 
			rlzRsrcOfChains (chainsThatLeftDatacenter);

			initAlg ();
			// Schedule a self-event for reading the handling the next time-step
			scheduleAt (simTime() + period, new cMessage); 
			break;
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
	runTimePeriod ();
}

void SimController::finish () 
{
  traceFile.close ();
	if (LOG_LVL>0) {
  	MyConfig::printToLog ("\nfinished sim\n");
  }
}

/*************************************************************************************************************************************************
- Inc. numMigs for every chain where curDC!=nxtDc.
- If running in sync mode: calculate and print the total cost
**************************************************************************************************************************************************/
void SimController::concludeTimePeriod ()
{
	int numMigs;
	if (!ChainsMaster::concludeTimePeriod (numMigs)) {
		error ("error occured during run of ChainsMaster::concludeTimePeriod. Check log file for details.");
	}
	
	if (DEBUG_LVL > 0) {
		checkChainsMasterData ();
	}
	int nonMigCost = ChainsMaster::calcNonMigCost ();
	if (nonMigCost < 0) {
		error ("t=%d ChainsMaster::calcNonMigCost returned a negative number. Check log file for details.");
	}

	int periodCost = numMigs * uniformChainMisgCost + nonMigCost;
	
		if (RES_LVL > 0) {
		snprintf (buf, bufSize, "\nt=%d, tot cost = %d", t, periodCost);
		printBufToRes();
	}

	if (LOG_LVL>=BASIC_LOG) {
		MyConfig::printToLog ("\nBUPU results:");
		printAllDatacenters ();
		if (DEBUG_LVL>1) {
			MyConfig::printToLog ("\nBy ChainsMaster:\n");
			ChainsMaster::printAllDatacenters (numDatacenters);
		}
	}
	
	chainsThatJoinedLeaf.    clear ();
	chainsThatLeftDatacenter.clear ();
	fill(rcvdFinishedAlgMsgFromLeaves.begin(), rcvdFinishedAlgMsgFromLeaves.end(), false);
}


// print all the placed (and possibly, the pot-placed) chains on each DC by the datacenter's data.
void SimController::printAllDatacenters (bool printPotPlaced, bool printPushUpList, bool printInCntrFormat)
{
	for (const auto datacenter : datacenters) {
		datacenter -> print (printPotPlaced, printPushUpList, printInCntrFormat);
	}
}

// parse a token of the type "u,poa" where u is the chainId number and poas is the user's current poa
void SimController::parseChainPoaToken (string const token, ChainId_t &chainId, DcId_t &poaId)
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
- insert all the IDs of chains that left some datacenter dc to chainsThatLeftDatacenter[dc].
- remove all the chains whose users left from ChainsMaster::allChains.
Inputs:
- line: a string, containing a list of the IDs of the chains that left the simulated area.
**************************************************************************************************************************************************/
void SimController::rdUsrsThatLeftLine (string line)
{
  Chain chain; // will hold the new chain to be inserted each time
  ChainId_t chainId;
  DcId_t chainCurDc;
	vector <ChainId_t> usrsThatLeft; // the users that left at the current period
  char_separator<char> sep(" ");
  tokenizer<char_separator<char>> tokens(line, sep);
  
  // parse each old chain in the trace (.poa file), and find its current datacenter
	for (const auto& token : tokens) {
  	chainId = stoi (token);

		if (!ChainsMaster::findChain (chainId, chain)) { 
			error ("t=%d: didn't find chain id %d that left", t, chainId);
	  }
    
  	chainCurDc = chain.curDc;
  	if (DEBUG_LVL>0 && chainCurDc == UNPLACED_DC) {
			error ("Note: this chain was not placed before leaving\n"); 
  	}
		chainsThatLeftDatacenter[chainCurDc].push_back (chainId);  //insert the id of the moved chain to the vector of chains that left the current datacenter, where the chain is placed.
		usrsThatLeft.push_back (chainId);
  }
	if (!ChainsMaster::eraseChains (usrsThatLeft)){
		error ("t=%d: ChainsMaster::eraseChains didn't find a chain to delete.", t);
	}
	usrsThatLeft.clear ();
}

/*************************************************************************************************************************************************
* Determine whether the generated chain would be RT, or Non_RT.
* This is done based on either random selection, or pseudo-random, based on the chainId.
**************************************************************************************************************************************************/
bool SimController::genRtChain (ChainId_t chainId)
{
	if (randomlySetChainType) {
		return (rand () < RT_chain_rand_int);
	}
	else if (evenChainsAreRt) {
		return (chainId%2==0);
	}
	else {
		return (float(chainId % 10)/10) < RT_chain_pr;
	}
}			
			

/*************************************************************************************************************************************************
Read a trace line that includes data about new chains.
Generate a new chain, and add it to the ChainsMaster::allChains.
Also, add the new generated chain to chainsThatJoinedLeaf[leaf], where leaf is the curent leaf, co-located with the poa of this new chain (poa is indicated in the trace, .poa file).
Inputs: 
- line: the line to parse. The line contains data in the format (c_1, leaf_1)(c_2, leaf_2), ... where leaf_i is the updated PoA of chain c_i.
**************************************************************************************************************************************************/
void SimController::rdNewUsrsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  ChainId_t chainId;
  DcId_t 		poaId; 
	Chain chain; // will hold the new chain to be inserted each time
  
	for (const auto& token : tokens) {
		parseChainPoaToken (token, chainId, poaId);
		
		if (genRtChain(chainId)) {
			vector<DcId_t> S_u = {pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+RT_Chain::mu_u_len};
			chain = RT_Chain (chainId, S_u); 		
		}
		else {
			vector<DcId_t> S_u = {pathToRoot[poaId].begin(), pathToRoot[poaId].begin()+Non_RT_Chain::mu_u_len};
			chain = Non_RT_Chain (chainId, S_u); 
		}
		
		if (DEBUG_LVL>1 && ChainsMaster::findChain (chainId, chain)){
			error ("t=%d: in rdNewUsrsLine, new chain %d already found in allChains\n", t, chainId);
		}
		
		insertSorted (chainsThatJoinedLeaf[poaId], chain); // insert the chain to its correct order in the (ordered) vector of chainsThatJoinedLeaf[poaId].		
		if (!ChainsMaster::insert (chainId, chain)) {
			error ("t=%d new chain %d was already found in ChainsMaster", t, chainId);
		}
	}	
	
}

/*************************************************************************************************************************************************
- Read a trace line that includes data about old chains, that moved and thus became critical.
- Find the chain in the db "ChainsMaster::allChains". 
- insert the chain to chainsThatJoinedLeaf[leaf], where leaf is the new, updated leaf.
Inputs:
- line: the line to parse. The line contains data in the format (c_1, leaf_1)(c_2, leaf_2), ... where leaf_i is the updated leaf of chain c_i.
**************************************************************************************************************************************************/
void SimController::rdOldUsrsLine (string line)
{
  char_separator<char> sep("() ");
  tokenizer<char_separator<char>> tokens(line, sep);
  ChainId_t chainId;
  DcId_t poaId;
	Chain chain; 

	for (const auto& token : tokens) {
		parseChainPoaToken (token, chainId, poaId);
  	chainId = stoi (token);
  	
  	if (!ChainsMaster::modifyS_u (chainId, pathToRoot[poaId], chain))
  	{
			error ("t=%d: old chain id %d is not found, or not placed", t, chainId);  	
  	}
		if (DEBUG_LVL>0 && chain.curDc == UNPLACED_DC) {
			error ("t=%d: at rdOldUsrsLine, old usr %d wasn't placed yet\n", t, chainId);
		}
		if (chain.curLvl==UNPLACED_LVL) { // if the current place of this chain isn't delay-feasible for it anymore
			insertSorted (chainsThatJoinedLeaf[poaId], chain); // need to inform the chain's new poa that it has to place it
			chainsThatLeftDatacenter[chain.curDc].push_back (chain.id); // need to rlz this chain's rsrcs from its current place
		}
	}

	
}


/*************************************************************************************************************************************************
- Call each datacenter to inform it of all chains that left it - due to either leaving the sim', moving to another poa, or due to a preparation
  for reshuffle.
**************************************************************************************************************************************************/
void SimController::rlzRsrcOfChains (unordered_map <DcId_t, vector<ChainId_t> > &ChainsToRlzFromDc) 
{

	for (auto &item : ChainsToRlzFromDc) // each item in the list includes dcId, and a list of chains that left the datacenter with this dcId.
	{
		datacenters[item.first]->rlzRsrc (item.second); 
	}

}

// Initiate the run of placement alg'
void SimController::initAlg () {  	

	return (mode==SYNC)? initAlgSync() : initAlgAsync();
}

/*************************************************************************************************************************************************
Compare the chainsMaster's chains' location data to the datacenters' placedChains data.
Raise an error in case of data inconsistency.
**************************************************************************************************************************************************/
void SimController::checkChainsMasterData ()
{
	Chain chain;
	for (DcId_t dcId=0; dcId<numDatacenters; dcId++) {
		for (auto chainId : datacenters[dcId]->placedChains) {
			if (!ChainsMaster::findChain (chainId, chain)) {
				error ("\nchain %d found in DC's %d placedChain isn't found in ChainsMaster", chainId, dcId);
			}
			if (chain.curDc != dcId) {
				error ("\nchain %d found in DC's %d placedChain is recorded in ChainsMaster as placed on DC %d", chainId, dcId, chain.curDc);
			}
		}
	}
}


/*************************************************************************************************************************************************
Prepare a reshuffle. This function is invoked separately (using a direct msg) be each leaf (poa) that takes part in a reshuffle.
The function does the following:
- rlz the rsrscs of all the chains associated with this poa.
- Initiate a placement alg' from this poa, where notAssigned=all users currently associated with this PoA.
**************************************************************************************************************************************************/
void SimController::prepareReshSync (DcId_t dcId, DcId_t leafId)
{

  unordered_map <DcId_t, vector<ChainId_t> > chainsToReplace;
  vector<Chain> vecOfUsrsOfThisPoA; 

	for (const auto &it : ChainsMaster::allChains) {
		if (it.second.S_u[0] == dcId) { // if the dcId of the chain's poa is the src of the msg that requested to prepare a sync resh...
			DcId_t chainCurDatacenter = (it.second).curDc;
			if (chainCurDatacenter != UNPLACED_DC) { // if this chain isn't already placed, no need to release it.
				chainsToReplace[chainCurDatacenter].push_back (it.first); // If this chain's PoA is the leaf that requested resh, the chain should be released
			}
			insertSorted (vecOfUsrsOfThisPoA, it.second);
		}
	}
	rlzRsrcOfChains (chainsToReplace);
	if (LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nSimCtrlr calling DC %d.initBottomUp with vecOfChainsThatJoined=", dcId);
		printBufToLog ();
		MyConfig::printToLog (vecOfUsrsOfThisPoA);
	}
	datacenters[dcId]->initBottomUp (vecOfUsrsOfThisPoA);
}

// Initiate the run of an Sync placement alg'
void SimController::initAlgSync () 
{  	

	if (LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\nt=%d, initiating alg.", t);
		printBufToLog();
	} 

	bool *initAlgAtLeaf { new bool[numLeaves]{} }; // initAlgAtLeaf[i] will be true iff we already initiated a run of BUPU in leaf i

	// First, initiate the alg' in all the leaves to which new chains have joined.
	for (auto item : chainsThatJoinedLeaf)
	{
		if (LOG_LVL==2) {
			logFile << "Chains that joined dc " << item.first << ": ";
		}
		leaves[item.first]->initBottomUp (item.second);
		initAlgAtLeaf[item.first] = true;
	}

	// Next, initiate the alg' in the remainder leaves, just to initiate sync' BUPU.
	for (DcId_t leafId(0); leafId < numLeaves; leafId++) {
		if (!(initAlgAtLeaf[leafId])) {
			vector<Chain> emptyVecOfChains = {};
			leaves[leafId]->initBottomUp (emptyVecOfChains);
		}
	}	
	delete[] initAlgAtLeaf;
}

// Initiate the run of a Async placement alg'
void SimController::initAlgAsync () 
{

}

/*************************************************************************************************************************************************
This func' is called on sync' mode, when a leaf finishes the alg'. The func'
- Increase the cntrs of the number of migs as required.
- Update ChainsMaster::allChains db.
- If all leaves finished, conclude this time step.
**************************************************************************************************************************************************/
void SimController::finishedAlg (DcId_t dcId, DcId_t leafId)
{

	Enter_Method ("finishedAlg (DcId_t dcId, DcId_t)");
	if (DEBUG_LVL>0 && mode==ASYNC) {
		error ("t = %d DC %d called finishedAlg in Async mode", t, dcId);
	}
	rcvdFinishedAlgMsgFromLeaves [leafId] = true; 
	if (LOG_LVL>=VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nrcvd fin alg msg from DC %d leaf %d", dcId, leafId);
		MyConfig::printToLog (buf);
	}
	
	bool rcvdFinishedAlgMsgFromAllLeaves = true;
	for (int i(0); i < numLeaves; i++) {
		if (!rcvdFinishedAlgMsgFromLeaves[i]) {
			rcvdFinishedAlgMsgFromAllLeaves = false;
		}
	}
	if (rcvdFinishedAlgMsgFromAllLeaves) {

		if (LOG_LVL>=DETAILED_LOG) {
			MyConfig::printToLog ("\nrcvd fin alg msg from all leaves");
		}
		
		std::fill(rcvdFinishedAlgMsgFromLeaves.begin(), rcvdFinishedAlgMsgFromLeaves.end(), false);
	}
	
}


/*************************************************************************************************************************************************
Print the "state" (PoAs of active users, and current placement of chains), and end the sim'.
**************************************************************************************************************************************************/
void SimController::PrintStateAndEndSim () 
{
	Enter_Method ("PrintStateAndEndSim ()");
	MyConfig::printToLog ("Printing state and finishing simulation\n");
	printAllDatacenters (true, false);
	if (LOG_LVL >= DETAILED_LOG) {
		MyConfig::printToLog ("Printing the PoAs of each chain\n");
  	ChainsMaster::printAllChainsPoas  (); 
  }
	MyConfig::printToLog ("simulation abnormally terminated by SimController.PrintStateAndEndSim");
	error ("simulation abnormally terminated by SimController.PrintStateAndEndSim. Check the log file for details.");
}

void SimController::handleMessage (cMessage *msg)
{
  if (msg -> isSelfMessage()) {
		isFirstPeriod = false;
		if (!isLastPeriod) {
			runTimePeriod ();
		}
  }
  else if (dynamic_cast<PrintAllDatacentersMsg*> (msg)) { 
  	printAllDatacenters ();
  }
  else if (dynamic_cast<PrintStateAndEndSimMsg*> (msg)) { 
  	PrintStateAndEndSim ();
  }
  else {
  	error ("Rcvd unknown msg type");
  }
  delete (msg);
}

