/*************************************************************************************************************************************************
Controller of the simulation:
- reads the trace.
- runs ths placing algorithm, by calling the relevant datacenter.
- keeps tracks of the placing algorithm's results and costs.
**************************************************************************************************************************************************/
#include "SimController.h"

class Datacenter;
bool  MyConfig::notifiedReshInThisPeriod;
float MyConfig::RtChainPr; // prob' that a new chain is an RT chain
bool  MyConfig::randomlySetChainType = false;
bool  MyConfig::evenChainsAreRt;
char 	MyConfig::modeStr[12]; 
Lvl_t MyConfig::lvlOfHighestReshDc;
Cpu_t MyConfig::cpuAtLeaf;
bool  MyConfig::discardAllMsgs;
int   MyConfig::LOG_LVL;
bool MyConfig::printBuRes; // when true, print to the log and to the .res file the results of the BU stage of BUPU
vector <Cpu_t> MyConfig::cpuAtLvl; 

// returns true iff the given datacenter dcId, at the given level, is delay-feasible for this chain (namely, appears in its S_u)

Define_Module(SimController);

//returns the index of the Dc of level lvl in the path from Dc i to the root
inline Lvl_t  SimController::idxInpathFromDcToRoot (DcId_t i, Lvl_t lvl) {return lvl - datacenters[i]->lvl;}

// convert leafId <--> dcId
inline DcId_t SimController::leafId2DcId (DcId_t leafId) {return leaves		 [leafId]->dcId;}
inline DcId_t SimController::dcId2leafId (DcId_t dcId)   {return datacenters[dcId]  ->leafId;}

SimController::SimController() {
}

SimController::~SimController() {}

void SimController::initialize (int stage)
{

  if (stage==0) {
		network         = (cModule*) (getParentModule ()); // No "new", because then need to dispose it.
		networkName 		= (network -> par ("name")).stdstringValue();
		MyConfig::setNetTypeFromString (networkName);
		if (MyConfig::netType<0) {		
			printErrStrAndExit ("The .ini files assigns network.name=" + networkName + " This network name is currently not supported ");
		}
		numDatacenters  = (DcId_t) (network -> par ("numDatacenters"));
		numLeaves       = (DcId_t) (network -> par ("numLeaves"));
		height       		= (Lvl_t) (network -> par ("height"));
		srand(seed); // set the seed of random num generation
		return;
	}
	
  if (stage==1) {
		snprintf (MyConfig::modeStr, 12, (mode==SYNC)? "Sync" : "Async"); 
		MyConfig::cpuAtLeaf = MyConfig::nonAugmentedCpuAtLeaf[MyConfig::netType];
		for (Lvl_t lvl(0); lvl < height; lvl++) {
			MyConfig::cpuAtLvl.push_back ((MyConfig::netType==UniformTreeIdx)? 1 : (MyConfig::cpuAtLeaf*(lvl+1)));
		}
		openFiles ();
		RtChain		::costAtLvl = MyConfig::RtChainCostAtLvl		 	[MyConfig::netType];
		NonRtChain::costAtLvl = MyConfig::NonRtChainCostAtLvl	[MyConfig::netType];
		RtChain::mu_u 		 			= MyConfig::RtChainMu_u 					[MyConfig::netType];
		NonRtChain::mu_u 			= MyConfig::NonRtChainMu_u 			[MyConfig::netType];
		RtChain	  ::mu_u_len 	= RtChain		::mu_u.size();
		NonRtChain::mu_u_len 	= NonRtChain::mu_u.size();
    RtChainRandInt 				= (int) (MyConfig::RtChainPr * (float) (RAND_MAX));//the max integer, for which we'll consider a new chain as a RTChain.

		// Set the prob' of a generated chain to be an RtChain
		if (MyConfig::netType==MonacoIdx || MyConfig::netType==LuxIdx) {
			MyConfig::evenChainsAreRt			 = false;
			MyConfig::RtChainPr = 0.3; // prob' that a new chain is an RT chain
		}
		else {
			MyConfig::RtChainPr = 0.5; // prob' that a new chain is an RT chain
			MyConfig::evenChainsAreRt			 = true;
		}

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
			else {
				datacenters[dc] -> setLeafId (-1);
			}
		}
		discoverPathsToRoot  ();
		calcDistBetweenAllDcs				 ();
		ChainsMaster::clear  ();
		return;
	}
	
	if (stage==2) {
		MyConfig::LOG_LVL=NO_LOG;
		MyConfig::printBuRes = true; // when true, print to the log and to the .res file the results of the BU stage of BUPU
		MyConfig::discardAllMsgs = false;
		runTrace ();
	}
}

void SimController::openFiles ()
{
	if (MyConfig::netType==MonacoIdx) {
		MyConfig::traceFileName = "Monaco_0730_0830_1secs_Telecom.poa";
	}
	else if (MyConfig::netType==LuxIdx) {
		MyConfig::traceFileName = "Lux_0730_0730_1secs_post.poa";  //"Lux_short.poa"; // 
	}
	else {
		MyConfig::traceFileName = "UniformTree.poa";
	}

	setOutputFileNames ();
	int traceNetType = MyConfig::getNetTypeFromString (MyConfig::traceFileName);
	if (traceNetType!=MyConfig::MyConfig::netType) {
		printErrStrAndExit ("traceFileName " + MyConfig::traceFileName + " doesn't correspond .ini fileName " + networkName + ".ini");
	}
	// Now, after stage 0 is done, we know that the network and all the datacenters have woken up.
	if (!MyConfig::openFiles ()) {
		error ("MyConfig::openFiles failed");
	}
}

void SimController::checkParams ()
{

	for (int lvl(0); lvl < RtChain::costAtLvl.size()-1; lvl++) {
		if ((int)(RtChain::costAtLvl[lvl]) <= (int)(RtChain::costAtLvl[lvl+1])) {
			error ("RtChain::costAtLvl[] should be decreasing. However, RtChain::costAtLvl[%d]=%d, RtChain::costAtLvl[%d]=%d\n", 
							lvl, RtChain::costAtLvl[lvl], lvl+1, RtChain::costAtLvl[lvl+1]);
		}
	}
	for (int lvl(0); lvl < RtChain::costAtLvl.size()-1; lvl++) {
		if ((int)(NonRtChain::costAtLvl[lvl]) <= (int)(NonRtChain::costAtLvl[lvl+1])) {
			error ("NonRtChain::costAtLvl[] should be decreasing. However, NonRtChain::costAtLvl[%d]=%d, NonRtChain::costAtLvl[%d]=%d\n", 
							lvl, NonRtChain::costAtLvl[lvl], lvl+1, NonRtChain::costAtLvl[lvl+1]);
		}
	}
}

/*************************************************************************************************************************************************
- Fill this->pathFromLeafToRoot. pathFromLeafToRoot[i] will hold the path from leaf i to the root.
- Fill this->pathFromDcToRoot. pathFromDcToRoot[i] will hold the path from Dc i to the root.
**************************************************************************************************************************************************/
void SimController::discoverPathsToRoot () {
	pathFromLeafToRoot.resize (numLeaves);
	DcId_t dcId;
	for (int leafId(0) ; leafId < numLeaves; leafId++)  {
		pathFromLeafToRoot[leafId].resize (height);
		dcId = leaves[leafId]->dcId;
	  int height = 0;
		while (dcId != root_id) {
		 	pathFromLeafToRoot[leafId][height++] = dcId;
		 	dcId = datacenters[dcId]->idOfParent;
		}
	}
	pathFromDcToRoot.resize (numDatacenters);
	for (int leafId(0) ; leafId < numLeaves; leafId++)  {
		for (Lvl_t lvl (0); lvl<height; lvl++) {
			for (Lvl_t i(0); lvl+i<height; i++) {
				dcId = pathFromLeafToRoot[leafId][lvl];
				if (pathFromDcToRoot[dcId].size() < height - datacenters[dcId]->lvl) { // this path is not full yet -- need to add DCs to it.
					pathFromDcToRoot[pathFromLeafToRoot[leafId][lvl]].push_back(pathFromLeafToRoot[leafId][lvl+i]);
				}
			}
		}
	}
}

/*************************************************************************************************************************************************
Returns the distance (in num of hops) between DC i and DC j
**************************************************************************************************************************************************/
Lvl_t SimController::dist (DcId_t i, DcId_t j) {
	if (i==j) {
		return 0;
	}
	return (i<j)? distTable[i][j-i] : distTable[j][i-j];
}

/*************************************************************************************************************************************************
Calculate the distance (in num of hops) beween node i and node j
**************************************************************************************************************************************************/
Lvl_t SimController::calcDistBetweenTwoDcs (DcId_t i, DcId_t j)
{
	if (datacenters[i]->lvl < datacenters[j]->lvl) {
		error ("calcDistBetweenTwoDcs cannot calculate the dist between i and j when j is in higher level than i");
	}
	for (Lvl_t lvl=datacenters[i]->lvl; lvl<height; lvl++) {
		if (pathFromDcToRoot[i][idxInpathFromDcToRoot(i, lvl)]==pathFromDcToRoot[j][idxInpathFromDcToRoot(j, lvl)]) {
			return idxInpathFromDcToRoot(i, lvl) + idxInpathFromDcToRoot(j, lvl);
		}
	}
	return -1;
} 

/*************************************************************************************************************************************************
Calculate the distance (in num of hops) between each pair of datacenters i, j, where i<j.
Write the distances to this->distTable.
**************************************************************************************************************************************************/

void SimController::calcDistBetweenAllDcs () {
	distTable.resize (numDatacenters);
	for (DcId_t i(0) ; i < numDatacenters; i++)  {
		for (DcId_t j=1; i+j < numDatacenters; j++) {
			Lvl_t dist = calcDistBetweenTwoDcs (i, i+j);
			if (dist < 0) {
				error ("couldn't calc the dist between Dc %d and Dc %d", i, i+j);
			}
			distTable[i].push_back (dist);
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
	
	MyConfig::notifiedReshInThisPeriod = false;

  string line;
  
  // discard empty and comment lines
  while (getline (traceFile, line)) { 
  	if (line.compare("")==0 || (line.substr(0,2)).compare("//")==0 ){ 
  		continue;
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

			if (MyConfig::LOG_LVL>0) {
				snprintf (buf, bufSize, "\n\nt = %d\n********************", t);
				MyConfig::printToLog (buf); 
			}
		}
		else if ( (line.substr(0,15)).compare("usrs_that_left:")==0) {			
			rdUsrsThatLeftLine (line.substr(16));

			// rlz the rsrcs of chains that left from their current datacenters 
			rlzRsrcOfChains (chainsThatLeftDatacenter);
			chainsThatLeftDatacenter.clear ();

			//Remove the left chains from ChainsMaster
			if (!ChainsMaster::eraseChains (usrsThatLeft)){
				error ("t=%d: ChainsMaster::eraseChains didn't find a chain to delete.", t);
			}
			usrsThatLeft.clear ();
		} 	
		else if ( (line.substr(0,9)).compare("new_usrs:")==0) {
			rdNewUsrsLine (line.substr(10)); 
		}
		else if ( (line.substr(0,9)).compare("old_usrs:")==0) {
			rdOldUsrsLine (line.substr(10));
			
			// rlz rsrcs of chains that left their current location 
			rlzRsrcOfChains (chainsThatLeftDatacenter);
			chainsThatLeftDatacenter.clear ();
		
			//$$$
			initAlg (); // call a placement algorithm 
			scheduleAt (simTime() + period, new RunTimePeriodMsg); // Schedule a self-event for reading the handling the next time-step
			break;
		}
  }
}

/*************************************************************************************************************************************************
* Print the given error msg, and call Omnet's "error" func' to exit with error.
**************************************************************************************************************************************************/
void SimController::printErrStrAndExit (const string &errorMsgStr)
{
	char errorMsg[errorMsgStr.length() + 1];
	strcpy(errorMsg, errorMsgStr.c_str());
	error (errorMsg); 
}

/*************************************************************************************************************************************************
* Run the whole trace
**************************************************************************************************************************************************/
void SimController::runTrace () {
	traceFile = ifstream (tracePath + MyConfig::traceFileName);
	
	isFirstPeriod 								= true;
  numMigsAtThisPeriod 					= 0; // will cnt the # of migrations in the current run
  MyConfig::lvlOfHighestReshDc  = UNPLACED_LVL;
  
  if (!traceFile.is_open ()) {
  	printErrStrAndExit ("trace file " + tracePath + MyConfig::traceFileName + " was not found");
  }
	runTimePeriod ();
}

void SimController::finish () 
{
  traceFile.close ();
	if (MyConfig::LOG_LVL>0) {
  	MyConfig::printToLog ("\nfinished sim\n");
  }
}

/*************************************************************************************************************************************************
* Print to log the overall cost after running the BU stage.
* This cost considers chains that are either placed, or only potentially-placed at each datacenter.
* That is, it assumes that each potentially-placed chain is already placed, and considers its cost accordingly.
**************************************************************************************************************************************************/
void SimController::printBuCost ()
{
	Enter_Method ("SimController::printBuCost ()");
	int 	nonMigCost = 0;
	Chain chain;

	for (DcId_t dcId=0; dcId < numDatacenters; dcId++) {

		for (const auto &chainId : datacenters[dcId]->placedChains) {
			if (!ChainsMaster::findChain (chainId, chain)) {
				error ("error in SimController::printBuCost");
			}
			nonMigCost += chain.getCostAtLvl (datacenters[dcId]->lvl);
			if (chain.curDc!=UNPLACED_DC && chain.curDc!=dcId) {
				numMigsAtThisPeriod++;
			}
		}
		for (const auto &chainId : datacenters[dcId]->potPlacedChains) {
			if (!ChainsMaster::findChain (chainId, chain)) {
				error ("error in SimController::printBuCost");
			}
			nonMigCost += chain.getCostAtLvl (datacenters[dcId]->lvl);
			if (chain.curDc!=UNPLACED_DC && chain.curDc!=dcId) {
				numMigsAtThisPeriod++;
			}
		}

	}
	snprintf (buf, bufSize, "\nt=%d, nonMigCost=%d, numMigs=%d, tot cost = %d", t, nonMigCost, numMigsAtThisPeriod, nonMigCost + numMigsAtThisPeriod * uniformChainMigCost);
	printBufToLog();
}

/*************************************************************************************************************************************************
- Call ChainsMaster to handle its own operation to conclude the time period.
- Calc the # of migrations at this period.
- If running in sync mode: calculate and print the total cost
**************************************************************************************************************************************************/
void SimController::concludeTimePeriod ()
{

	ChainId_t errChainId;
	int stts = ChainsMaster::concludeTimePeriod (numMigsAtThisPeriod, errChainId);
	
	if (stts!=0) {
		error ("sim t=%lf, t=%d: error during run of ChainsMaster::concludeTimePeriod. err type=%d. errChainId=%d. For further details, plz Check the log file.",
					  simTime().dbl(), t, stts, errChainId);
	}
	
	if (DEBUG_LVL > 0) {
		checkChainsMasterData ();
	}
	if (RES_LVL > 0) {
		 printResLine ();
	}

	MyConfig::printToLog ("\nBUPU results:"); // $$$
	printAllDatacenters (false, false, true); //$$$


	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		MyConfig::printToLog ("\nBUPU results:");
		printAllDatacenters (false, false, false); //$$$
		if (DEBUG_LVL>1) {
			MyConfig::printToLog ("\nBy ChainsMaster:\n");
			ChainsMaster::printAllDatacenters (numDatacenters);
		}
	}
	
	// reset state variables, in preparation for the next period
	chainsThatJoinedLeaf.clear ();
	fill(rcvdFinishedAlgMsgFromLeaves.begin(), rcvdFinishedAlgMsgFromLeaves.end(), false);
	numMigsAtThisPeriod = 0; 
	numCritUsrs					= 0;
	MyConfig::lvlOfHighestReshDc=UNPLACED_LVL;
}


// print all the placed (and possibly, the pot-placed) chains on each DC by the datacenter's data.
void SimController::printAllDatacenters (bool printPotPlaced, bool printPushUpList, bool printChainIds)
{
	if (printPotPlaced) {
		MyConfig::printToLog ("\n************* Note: this print includes also potPlaced chains *************");
	}
	for (const auto datacenter : datacenters) {
		datacenter -> print (printPotPlaced, printPushUpList, printChainIds);
	}
}

/*************************************************************************************************************************************************
Read and handle a trace line that details the IDs of chains that left the simulated area.
- insert all the IDs of chains that left some datacenter dc to this->chainsThatLeftDatacenter[dc].
- insert all the IDs of chains that left ANY dc to this->usrsThatLeft.
Inputs:
- line: a string, containing a list of the IDs of the chains that left the simulated area.
Note: this func' still does NOT remove the left chains from ChainsMaster::allChains, because the data about the left chains (in particular, 
the currently used rsrc) is needed for letting the currently-hosting DCs to regain the rsrc.
each chain that left will be removed from ChainsMaster::allChains only after it's removed its curDc.
**************************************************************************************************************************************************/
void SimController::rdUsrsThatLeftLine (string line)
{
  Chain chain; // will hold the left chain
  ChainId_t chainId;
  DcId_t chainCurDc;
  stringstream streamOfChainIds (line);
  
  // parse each old chain in the trace (.poa file), and find its current datacenter
  while (streamOfChainIds >> chainId) {
		if (!ChainsMaster::findChain (chainId, chain)) { 
			error ("t=%d: didn't find chain id %d that left", t, chainId);
	  }    
  	chainCurDc = chain.curDc;
  	if (DEBUG_LVL>0 && chainCurDc == UNPLACED_DC) {
			error ("Note: this chain was not placed before leaving\n"); 
  	}
		chainsThatLeftDatacenter[chainCurDc].push_back (chainId);//insert the id of the moved chain to the vec of chains that left its curreht Dc
		usrsThatLeft.push_back (chainId);
  }
}

/*************************************************************************************************************************************************
* Determine whether the generated chain would be RT, or NonRT.
* This is done based on either random selection; or pseudo-random, based on the chainId.
**************************************************************************************************************************************************/
bool SimController::genRtChain (ChainId_t chainId)
{
	if (MyConfig::randomlySetChainType) {
		return (rand () < RtChainRandInt);
	}
	else if (MyConfig::evenChainsAreRt) {
		return (chainId%2==0);
	}
	else {
		return (float(chainId % 10)/10) < MyConfig::RtChainPr;
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
  ChainId_t chainId;
  DcId_t 		poaId; 
	Chain chain; // will hold the new chain to be generated each time

	replace_if(begin(line), end(line), [] (char x) { return ispunct(x); }, ' ');
	stringstream ss (line); 
	
	while (ss >> chainId) {
		ss >> poaId;
		if (genRtChain(chainId)) {
			vector<DcId_t> S_u = {pathFromLeafToRoot[poaId].begin(), pathFromLeafToRoot[poaId].begin()+RtChain::mu_u_len};
			chain = RtChain (chainId, S_u);

		}
		else {
			vector<DcId_t> S_u = {pathFromLeafToRoot[poaId].begin(), pathFromLeafToRoot[poaId].begin()+NonRtChain::mu_u_len};
			chain = NonRtChain (chainId, S_u); 
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
//  char_separator<char> sep("() ");
//  tokenizer<char_separator<char>> tokens(line, sep);
  ChainId_t chainId;
  DcId_t poaId;
	Chain chain; 
	
	replace_if(begin(line), end(line), [] (char x) { return ispunct(x); }, ' ');
	stringstream ss (line); 
	
	while (ss >> chainId) {
		ss >> poaId;	
  	if (!ChainsMaster::modifyS_u (chainId, pathFromLeafToRoot[poaId], chain))
  	{
			error ("t=%d: old chain id %d is not found, or not placed", t, chainId);  	
  	}
		if (DEBUG_LVL>0 && chain.curDc == UNPLACED_DC) {
			error ("t=%d: at rdOldUsrsLine, old usr %d wasn't placed yet\n", t, chainId);
		}
		if (chain.curLvl==UNPLACED_LVL) { // if the current place of this chain isn't delay-feasible for it anymore --> it's a critical chain
			numCritUsrs++;
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

	for (auto &item : ChainsToRlzFromDc) { // each item in the list includes dcId, and a list of chains that left the datacenter with this dcId.
		datacenters[item.first]->rlzRsrc (item.second); 
	}
}


/*************************************************************************************************************************************************
Initiate the run of placement alg'
**************************************************************************************************************************************************/
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
Initiates a full reshuffle. 
The function does the following:
- rlx the rsrscs of all the chains in the system.
- Initiate a placement alg' from all the leaves.
**************************************************************************************************************************************************/
void SimController::initFullReshSync ()
{

	if (MyConfig::LOG_LVL >= DETAILED_LOG) {
		MyConfig::printToLog ("\nbeginning full resh\n");
	}
	MyConfig::lvlOfHighestReshDc = height-1;
	
	// rlz all chains throughout
	for (DcId_t dcId=0; dcId<numDatacenters; dcId++) {
		datacenters[dcId]->clrRsrc();
		datacenters[dcId]->numBuPktsRcvd = 0;
	}
	
	chainsThatJoinedLeaf.clear ();
	for (auto &it : ChainsMaster::allChains) {
		DcId_t leafId = datacenters[it.second.S_u[0]]->leafId;
		if (leafId >= numLeaves || leafId<0) {
			snprintf (buf, bufSize, "\t=%d. error in initFullReshSync: chain %d has leafId=%d", t, it.second.id, leafId);
			printBufToLog();
			error ("Error in initFullReshSync. Please check the log file.");
		}
		insertSorted (chainsThatJoinedLeaf[leafId], it.second); // push the chain id into the vec' of chains that "joined" this usr's poa.
	}
	MyConfig::discardAllMsgs = false;
	ChainsMaster::displaceAllChains ();
	initAlgSync ();
}


/*************************************************************************************************************************************************
Prepare a full reshuffle. 
The function does the following:
- rlx the rsrscs of all the chains in the system.
- Initiate a placement alg' from all the leaves.
**************************************************************************************************************************************************/
void SimController::prepareFullReshSync ()
{
	Enter_Method ("prepareFullReshSync ()");
	MyConfig::discardAllMsgs = true;
	scheduleAt (simTime() + CLEARNACE_DELAY, new InitFullReshMsg); 
}


/*************************************************************************************************************************************************
Prepare a reshuffle. This function is invoked separately (using a direct msg) by each leaf (poa) that takes part in a reshuffle.
The function does the following:
- rlz the rsrscs of all the chains associated with this poa.
- Initiate a placement alg' from this poa, where notAssigned=all users currently associated with this PoA.
**************************************************************************************************************************************************/
void SimController::prepareReshSync (DcId_t dcId, DcId_t leafId)
{

	if (!	MyConfig::notifiedReshInThisPeriod && MyConfig::LOG_LVL>=BASIC_LOG) {
		MyConfig::printToLog ("\nresh");
		MyConfig::notifiedReshInThisPeriod = true;
	}

  unordered_map <DcId_t, vector<ChainId_t> > chainsToReplace;
  vector<Chain> vecOfUsrsOfThisPoA; 

	for (const auto &it : ChainsMaster::allChains) {
		if (it.second.S_u[0] == dcId) { // if the chain's poa is the src of the msg that requested to prepare a sync resh...
			DcId_t chainCurDatacenter = (it.second).curDc;
			if (chainCurDatacenter != UNPLACED_DC) { // if this chain isn't already placed, no need to release it.
				chainsToReplace[chainCurDatacenter].push_back (it.first); // If this chain's PoA is the leaf that requested resh, the chain should be released
			}
			insertSorted (vecOfUsrsOfThisPoA, it.second);
		}
	}
	rlzRsrcOfChains (chainsToReplace);
	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nSimCtrlr calling DC %d.initBottomUp with vecOfChainsThatJoined=", dcId);
		printBufToLog ();
		MyConfig::printToLog (vecOfUsrsOfThisPoA);
	}
	datacenters[dcId]->initBottomUp (vecOfUsrsOfThisPoA);
}

// Initiate the run of an Sync placement alg'
void SimController::initAlgSync () 
{  	

	for (DcId_t dcId(0); dcId < numDatacenters; dcId++) {
		datacenters[dcId]->numBuPktsRcvd = 0;
	}

	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\nt=%d, initiating alg.", t);
		printBufToLog();
	} 

	bool *initAlgAtLeaf { new bool[numLeaves]{} }; // initAlgAtLeaf[i] will be true iff we already initiated a run of BUPU in leaf i

	// First, initiate the alg' in all the leaves to which new chains have joined.
	for (auto item : chainsThatJoinedLeaf)
	{
		if (MyConfig::LOG_LVL==2) {
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
	if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
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

		if (MyConfig::LOG_LVL>=DETAILED_LOG) {
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
	MyConfig::printToLog ("\nPrinting state and finishing simulation\n");
	printAllDatacenters (true, false);
	if (MyConfig::LOG_LVL >= DETAILED_LOG) {
		MyConfig::printToLog ("Printing the PoAs of each chain\n");
  	ChainsMaster::printAllChainsPoas  (); 
  }
	MyConfig::printToLog ("\nsimulation terminated by SimController.PrintStateAndEndSim");
	error ("simulation abnormally terminated by SimController.PrintStateAndEndSim. Check the log file for details.");
}

void SimController::handleMessage (cMessage *msg)
{
  if (dynamic_cast<RunTimePeriodMsg*> (msg)) {
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
  else if (dynamic_cast<InitFullReshMsg*> (msg)) { 
  	initFullReshSync ();
  }
  else {
  	error ("Rcvd unknown msg type");
  }
  delete (msg);
}

/*************************************************************************************************************************************************
 * Writes to self.buf a string, detailing the sim' parameters (time, amount of CPU at leaves, probability of RT app' at leaf, status of the solution)
*************************************************************************************************************************************************/
inline void SimController::genSettingsBuf ()
{																																																					 
  snprintf (settingsBuf, settingsBufSize, "t%d_%s_cpu%d_p%.1f_sd%d_stts%d",	t, MyConfig::modeStr, MyConfig::cpuAtLeaf, MyConfig::RtChainPr, seed, stts); 
}

/*************************************************************************************************************************************************
 * Print a solution for the problem to the output res file.
*************************************************************************************************************************************************/
void SimController::printResLine ()
{
	genSettingsBuf ();
	MyConfig::printToRes (settingsBuf);
	int periodNonMigCost = ChainsMaster::calcNonMigCost ();
	if (periodNonMigCost < 0) {
		error ("t=%d ChainsMaster::calcNonMigCost returned a negative number. Check log file for details.");
	}

	int periodMigCost 	= numMigsAtThisPeriod * uniformChainMigCost;
	int periodLinkCost  = 0;  // link cost is used merely a place-holder, for backward-compitability with the res format used in (centralized) "SFC_migration".
	int periodTotalCost = periodNonMigCost + periodMigCost;
  snprintf (buf, bufSize, " | cpu_cost=%d | link_cost = %d | mig_cost=%d | tot_cost=%d | ratio=[%.2f %.2f %.2f] | num_usrs=%d | num_crit_usrs=%d | resh=%d\n", 
  					periodNonMigCost, periodLinkCost, periodMigCost, periodTotalCost,
  					float(periodNonMigCost)/float(periodTotalCost), float(periodLinkCost)/float(periodTotalCost), float(periodMigCost)/float(periodTotalCost), 
  					(int)ChainsMaster::allChains.size(), numCritUsrs, MyConfig::lvlOfHighestReshDc);
  printBufToRes ();
}


/*************************************************************************************************************************************************
 * Set the output file names based on the settings of this sim run
*************************************************************************************************************************************************/
void SimController::setOutputFileNames ()
{
	
	string traceRawName = MyConfig::traceFileName.substr(0, MyConfig::traceFileName.find_last_of("."));
	MyConfig::resFileName = traceRawName + ".res";
	MyConfig::logFileName = traceRawName + ".log";
}


















