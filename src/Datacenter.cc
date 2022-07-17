#include "Datacenter.h"

using namespace omnetpp;
using namespace std;

Define_Module(Datacenter);
/*************************************************************************************************************************************************
 * Infline functions
*************************************************************************************************************************************************/
inline bool sortChainsByCpuUsage (Chain lhs, Chain rhs) {return lhs.getCpu() <= rhs.getCpu();}

inline bool Datacenter::cannotPlaceThisChainHigher (const Chain chain) const {return chain.mu_u_len() <= this->lvl+1;}
inline bool Datacenter::canPlaceThisChainHigher 	 (const Chain chain) const {return !cannotPlaceThisChainHigher(chain);}

inline Cpu_t Datacenter::requiredCpuToLocallyPlaceChain (const Chain chain) const {return chain.mu_u_at_lvl(lvl);}

inline Cpu_t Datacenter::requiredCpuToPlaceChainAtLvl (const Chain chain, const Lvl_t lvl) const {return chain.mu_u_at_lvl(lvl);}

// Given the number of a child (0, 1, ..., numChildren-1), returns the port # connecting to this child.
inline Lvl_t Datacenter::portToChild (const Lvl_t child) const {if (isRoot) return child; else return child+1;} 

inline void Datacenter::sndDirectToSimCtrlr (cMessage* msg) {sendDirect (msg, simController, "directMsgsPort");}

inline void	Datacenter::printStateAndEndSim () { sndDirectToSimCtrlr (new PrintStateAndEndSimMsg);}

inline void Datacenter::regainRsrcOfChain (const Chain chain) {availCpu += chain.mu_u_at_lvl(lvl); }

inline bool Datacenter::withinAnotherResh (const Lvl_t reshInitiatorLvl) const 
{
	return (this->reshInitiatorLvl!=UNPLACED_LVL && this->reshInitiatorLvl!=reshInitiatorLvl);
}

inline bool Datacenter::IAmTheReshIniator () const
{
	return (this->reshInitiatorLvl == this->lvl);
}

Datacenter::Datacenter()
{
}

Datacenter::~Datacenter()
{
  for (int i(0); i < numPorts; i++) {
    if (endXmtEvents[i] != nullptr) {
      cancelAndDelete (endXmtEvents[i]);
    }
  }
}

/*************************************************************************************************************************************************
returns true iff curHandledMsg arrived from my prnt
*************************************************************************************************************************************************/
bool Datacenter::arrivedFromPrnt ()
{
	if (isRoot) {
		return false;
	}
	
	return curHandledMsg->arrivedOn (prntGateId); //("port$0");
}

void Datacenter::initialize(int stage)
{
	if (stage==0) {
		network     	= (cModule*) (getParentModule ()); 
		simController = (SimController*) network->getSubmodule("sim_controller");
		networkName 	= (network -> par ("name")).stdstringValue();
		numChildren 	= (Lvl_t)  (par("numChildren"));
		numParents  	= (Lvl_t)  (par("numParents"));
		lvl				  	= (Lvl_t)  (par("lvl"));
		dcId					= (DcId_t) (par("dcId"));
		numBuPktsRcvd = 0;
		reshInitiatorLvl = UNPLACED_LVL;

		numPorts    = numParents + numChildren;
		isRoot      = (numParents==0);
		isLeaf      = (numChildren==0);
		outputQ.        resize (numPorts);
		xmtChnl.        resize (numPorts); // the xmt chnl towards each neighbor
		endXmtEvents.   resize (numPorts);
		idOfChildren.   resize (numChildren);
		
		// Discover the xmt channels to the neighbors, and the neighbors' id's.
		for (int portNum (0); portNum < numPorts; portNum++) {
			cGate *outGate    = gate("port$o", portNum);
			xmtChnl[portNum]  = outGate->getTransmissionChannel();
			cModule *nghbr    = outGate->getNextGate()->getOwnerModule();
			if (isRoot) {
			  idOfChildren[portNum] = DcId_t (nghbr -> par ("dcId"));
			}
			else {
			  if (portNum==0) { // port 0 is towards the parents
			    idOfParent = DcId_t (nghbr -> par ("dcId"));
			  }
			  else { // ports 1...numChildren are towards the children
			    idOfChildren[portNum-1] = DcId_t (nghbr -> par ("dcId"));
			  }
			}       
		}
		if (!isRoot) {
			prntGateId = gate("port$i", 0)->getId();
		}
		fill(endXmtEvents. begin(), endXmtEvents. end(), nullptr);
		return;
	}
	
	// parameters that depend upon MyConfig can be initialized only after stage 0, in which MyConfig is initialized.
	cpuCapacity   = MyConfig::cpuAtLvl[lvl]; 
  availCpu    	= cpuCapacity; // initially, all cpu rsrcs are available (no chain is assigned)
  if (MyConfig::mode==Async) {
		rstReshAsync ();
		endFModeEvent  = nullptr;
		isInFMode 		 = false;
  }

}

/*************************************************************************************************************************************************
 * Print to the log file data about chains on this DC. 
 * Inputs: 
 * - printChainIds: when true, print in a format similar to that used in the centralized alg'.
 * - printPotPlaced: when true and there're potPlacedChains to print - print them.
 * - printPushUpList: when true and pushUpList isn't impty - print them.
 * - beginWithNewLine: when true, the print begins in a new line
 * - printNotAssigned: when true, print also the notAssigned list
*************************************************************************************************************************************************/
void Datacenter::print (bool printPotPlaced, bool printPushUpList, bool printChainIds, bool beginWithNewLine)
{

	if (placedChains.empty() && (!printPotPlaced || potPlacedChains.empty()) && (!printPushUpList || pushUpList.empty())) {
		return;
	}
	if (beginWithNewLine) {
		MyConfig::printToLog ("\n");
	}
	snprintf (buf, bufSize, "s%d : Rcs=%d, a=%d, used cpu=%d, num_of_placed_chains=%d", 
														dcId, cpuCapacity, availCpu, cpuCapacity-availCpu, int(placedChains.size()) );
	printBufToLog ();
	if (printChainIds) {
		MyConfig::printToLog (" chains [");
		MyConfig::printToLog (placedChains);	
		MyConfig::printToLog ("] ");
	}

	if (printPotPlaced) {
		MyConfig::printToLog ("potPlaced=[");
		MyConfig::printToLog (potPlacedChains);
		MyConfig::printToLog ("] ");
	}
	if (printPushUpList) {
		MyConfig::printToLog ("pushUpList: ");
		MyConfig::printToLog (pushUpList, false);
	}
}

void Datacenter::setLeafId (DcId_t leafId)
{
	this->leafId = leafId;
}


/*************************************************************************************************************************************************
 * Handle an arriving EndXmtMsg, indicating the end of the transmission of a pkt.
 * If the relevant output queue isn't empty, the function transmits the pkt in the head of the queue.
*************************************************************************************************************************************************/
void Datacenter::handleEndXmtMsg ()
{
  EndXmtMsg *endXmtMsg = (EndXmtMsg*) curHandledMsg;
  DcId_t portNum 		 = endXmtMsg -> getPortNum();
  endXmtEvents[portNum] = nullptr;
  if (outputQ[portNum].isEmpty()) {
      return;
  }

  // Now we know that the output Q isn't empty --> Pop and xmt the HoL pkt
  cPacket* pkt2snd = (cPacket*) outputQ[portNum].pop();
  xmt (portNum, pkt2snd);
}


void Datacenter::handleMessage (cMessage *msg)
{

  curHandledMsg = msg;
  if (dynamic_cast<EndXmtMsg*>(curHandledMsg) != nullptr) {
  	handleEndXmtMsg ();
  }
  else if (dynamic_cast<EndFModeMsg*>(curHandledMsg) != nullptr) {
  	if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
  		snprintf (buf, bufSize, "s%d exiting F mode", dcId);
  		printBufToLog ();
  	}
  	isInFMode     = false;
  	endFModeEvent = nullptr;
  }
	else if (MyConfig::discardAllMsgs) {
		delete curHandledMsg;
		return;
	}
  else if (dynamic_cast<BottomUpPkt*>(curHandledMsg) != nullptr) {
  	if (MyConfig::mode==Sync) { 
  		handleBottomUpPktSync();
  	} 
		else {
			if (isInFMode) {
				handleBottomUpPktAsyncFMode ();
			}
			else {
				handleBottomUpPktAsync ();
			}
		} 
  }
  else if (dynamic_cast<PushUpPkt*>(curHandledMsg) != nullptr) {
  	handlePushUpPkt ();
  }
  else if (dynamic_cast<PrepareReshSyncPkt*>(curHandledMsg) != nullptr)
  {
    if (MyConfig::mode==Sync) { 
    	prepareReshSync ();
    }
    else {
    	reshAsync();
    }
  }
  else if (dynamic_cast<ReshAsyncPkt*>(curHandledMsg) != nullptr)
  {
  	if (arrivedFromPrnt ()) {
	  	handleReshAsyncPktFromPrnt ();
	  }
	  else {
	  	handleReshAsyncPktFromChild ();
	  }
  }
  else
  {
    error ("rcvd a pkt of an unknown type");
  }
  delete (curHandledMsg);
}

/*************************************************************************************************************************************************
release resources of chains that left "this".
- For each chain that left:
	- Remove the chain from the lists of placed chains, potPlaced and newlyPlaced chains.
	rlz all the cpu resources assigned to this chain.
*************************************************************************************************************************************************/
void Datacenter::rlzRsrc (vector<int32_t> IdsOfChainsToRlz) 
{

	for (auto chainId : IdsOfChainsToRlz) {

		// First, remove the chain from the list of potPlacedChains
		auto search = potPlacedChains.find (chainId);
		if (search!=potPlacedChains.end()) { 
			Chain chain;
			if (!ChainsMaster::findChain (chainId, chain)) {
				error ("pot-placed chain %d was not found in ChainMaster", chainId);
			}
			regainRsrcOfChain (chain);
			potPlacedChains.erase (chainId);
			continue; 
		}
		
		// Next, remove the chain from the list of placedChains
		search = placedChains.find (chainId);
		if (search!=placedChains.end()) { 
			Chain chain;
			if (!ChainsMaster::findChain (chainId, chain)) {
				error ("placed chain %d was not found in ChainMaster", chainId);
			}
			regainRsrcOfChain (chain);
			placedChains.erase (chainId);
		}

	}
}

/*************************************************************************************************************************************************
Initiate the bottomUpAlg:
- Clear this->pushUpList and this->notAssigned.
- Insert the chains  into this->notAssigned. The input vector is assumed to be already sorted by the delay tightness.
- Schedule a self-msg to call0 bottomUp, for running the BU alg'.
* Note: this func to be called only when the Datacenter is a leaf.
*************************************************************************************************************************************************/
void Datacenter::initBottomUp (vector<Chain>& vecOfChainsThatJoined)
{

	Enter_Method ("initBottomUp (vector<Chain>& vecOfChainsThatJoined)");

	if (!isLeaf) { 
		error ("Non-leaf s%d : was called by initBottomUp");
	}
	pushUpList.				clear ();	
	potPlacedChains.  clear ();
	notAssigned = vecOfChainsThatJoined;

 	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : rcvd vecOfChainsThatJoined=", dcId);
		printBufToLog (); 
		MyConfig::printToLog(vecOfChainsThatJoined);
		print (); 
	}
	bottomUp ();
}

/*************************************************************************************************************************************************
Handle a rcvd PushUpPkt:
- Read the data from the pkt to this->pushUpList.
- Call pushUp() for running the PU alg'.
*************************************************************************************************************************************************/
void Datacenter::handlePushUpPkt () 
{

  PushUpPkt *pkt = (PushUpPkt*) this->curHandledMsg;
	
	for (int i(0); i< (pkt->getPushUpVecArraySize()); i++) {
		if (!insertChainToList (pushUpList, pkt->getPushUpVec (i))) {
			error ("Error in insertChainToList. See log file for details");
		}
	}
	
	if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : rcvd PU pkt. pushUpList=", dcId);
		MyConfig::printToLog (pushUpList, false);
	}
	pushUp ();
}

/*************************************************************************************************************************************************
Run the PU Sync alg'. 
Assume that this->pushUpList already contains the relevant chains.
*************************************************************************************************************************************************/
void Datacenter::pushUp ()
{

	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : beginning PU. pushUpList=", dcId);
		printBufToLog ();
		MyConfig::printToLog (pushUpList, false);
	}
	reshuffled = false;
	
	// Find all chains that were pushed-up for me, and regain resources for them.
	for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); ) { // for each chain in pushUpList
		auto search = potPlacedChains.find (chainPtr->id);
		if (search==potPlacedChains.end()) { // If this chain doesn't appear in my potPlacedChains, nothing to do
			chainPtr++;
			continue;
		}	
		
		if (chainPtr->curLvl > (this->lvl) ) { // was the chain pushed-up?
			regainRsrcOfChain (*chainPtr); // Yes --> regain its resources
		}
		else { //the chain wasn't pushed-up --> need to locally place it
			placedChains.			insert (chainPtr->id);
			ChainsMaster::modifyLvl  (chainPtr->id, lvl); // inform ChainMaster about the chain's place 
		}
		potPlacedChains.erase (chainPtr->id);
		chainPtr = pushUpList.erase (chainPtr); // finished handling this chain pushUpList --> remove it from the pushUpList, and go on to the next chain
	}

	// Next, try to push-up chains of my descendants
	pushUpList.sort (SortChainsForPushUpList());
	// to make no mess and to keep the sort while iterating on pushUpList, insert all modified chains to pushedUpChains. Later, will unify it again with pushUpList
	list <Chain> pushedUpChains; 
	Cpu_t requiredCpuToLocallyPlaceThisChain;
	for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); ) {
		requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain (*chainPtr);
		if (chainPtr->curLvl >= lvl || // shouldn't push-up this chain either because it's already pushed-up by me/by an ancestor, or ... 
				requiredCpuToLocallyPlaceThisChain > availCpu || // because not enough avail' cpu for pushing-up, or ...
				!chainPtr->dcIsDelayFeasible (dcId, lvl) || // because I'm not delay-feasible for this chain, or ...
				chainPtr->curDc == chainPtr->S_u[chainPtr->curLvl]) {// the chain's suggested new place is identical to its current place, thus saving mig' cost
//					ChainsMaster::modifyLvl (chainPtr->id, chainPtr->curLvl); // inform ChainMaster about the chain's place 
					chainPtr++;
					continue;
		}
		else { // the chain is currently placed on a descendant, and I have enough place for this chain-->push up this chain to me
			availCpu 						-= requiredCpuToLocallyPlaceThisChain;
			Chain pushedUpChain  = *chainPtr; // construct a new chain to insert to placedChains, because it's forbidden to modify the chain in pushUpList
			pushedUpChain.curLvl = lvl;
			chainPtr 						 = pushUpList.erase (chainPtr); // remove the pushed-up chain from the list of potentially pushed-up chains; to be replaced by a modified chain
			placedChains.				 insert (pushedUpChain.id);
			ChainsMaster::modifyLvl  (pushedUpChain.id, lvl); // inform ChainMaster about the chain's place 
			pushedUpChains.insert (pushedUpChains.begin(), pushedUpChain);
		}
	}
	
	for (auto chainPtr=pushedUpChains.begin(); chainPtr!=pushedUpChains.end(); chainPtr++) {
		insertChainToList (pushUpList, *chainPtr);
	}
	
	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : finihsed PU.", dcId);
		printBufToLog ();
		print ();
	}

	if (isLeaf) {

		if (MyConfig::mode == Sync) {
			simController->finishedAlg (dcId, leafId);
			if (DEBUG_LVL > 0 && !pushUpList.empty()) {
					error ("pushUpList isn't empty after running pushUp() on a leaf");
			}
		}
		return; // finished; this actually concluded the run of the BUPU alg' for the path from me to the root
	}

	genNsndPushUpPktsToChildren ();
	
}

/*************************************************************************************************************************************************
Generate pushUpPkts, based on the data currently found in pushUpList, and xmt these pkts to all the children
*************************************************************************************************************************************************/
void Datacenter::genNsndPushUpPktsToChildren ()
{
	PushUpPkt* pkt;	 // the packet to be sent 
	
	for (Lvl_t child(0); child<numChildren; child++) { // for each child...
		pkt = new PushUpPkt;
		pkt->setPushUpVecArraySize (pushUpList.size ()); // default size of pushUpVec, for case that all chains in pushUpList belong to this child; will later shrink pushUpVec otherwise 
		int idxInPushUpVec = 0;
		for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); ) {	// consider all the chains in pushUpList
			if (chainPtr->S_u[lvl-1]==idOfChildren[child])   { /// this chain is associated with (the sub-tree of) this child
				pkt->setPushUpVec (idxInPushUpVec++, *chainPtr);
				chainPtr = pushUpList.erase (chainPtr);
			}
			else {
				chainPtr++;
			}
		}
		
		// shrink pushUpVec to its real size
		pkt->setPushUpVecArraySize (idxInPushUpVec);
		
		if (MyConfig::mode==Sync || idxInPushUpVec>0) { // In sync' mode, send a pkt to each child; in async mode - send a pkt only if its push-up vec isn't empty
			sndViaQ (portToChild(child), pkt); //send the pkt to the child
		}
		else {
			delete (pkt);
		}
	}
	if (DEBUG_LVL>0 && MyConfig::mode==Sync && !pushUpList.empty()) {
		error ("pushUpList not empty after sending PU pkts to all children");
	}
}

/************************************************************************************************************************************************
Running the BU alg' at "feasibility" Async mode
*************************************************************************************************************************************************/
void Datacenter::bottomUpFMode ()
{

	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : beginning BU-f. notAssigned=", dcId);
		printBufToLog ();
		MyConfig::printToLog (notAssigned);
	}

	sort (notAssigned.begin(), notAssigned.end(), SortChainsForNotAssignedList());
	for (auto chainPtr=notAssigned.begin(); chainPtr!=notAssigned.end(); ) {
		Cpu_t requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain(*chainPtr); 
		if (availCpu >= requiredCpuToLocallyPlaceThisChain) { // I have enough avail' cpu for this chain --> assign it
				availCpu -= requiredCpuToLocallyPlaceThisChain;				
				placedChains.		  insert  (chainPtr->id);
 				ChainsMaster::modifyLvl  (chainPtr->id, lvl); // inform ChainMaster about the chain's place 
				chainPtr = notAssigned.erase (chainPtr);
		}
		else { 
			if (canPlaceThisChainHigher(*chainPtr)) { // Am I the highest delay-feasible DC of this chain?
				chainPtr++; //No enough availCpu for this chain, but it may be placed above me --> go on to the next notAssigned chain  
				continue;
			}
			
			// Not enough availCpu for this chain, and it cannot be placed higher
			if (reshuffled) {
				if (chainPtr -> isNew()) { // Failed to place a new chain even after resh
					MyConfig::overallNumBlockedUsrs++;
					error ("sorry. blocking chains isn't supported yet");
					if (!ChainsMaster::blockChain (chainPtr->id)) {
						error ("s%d tried to block chain %d that wasn't found in ChainsMaster", dcId, chainPtr->id);
					}
					chainPtr = notAssigned.erase (chainPtr); 
				}
				else { // Failed to place an old chain even after resh
					snprintf (buf, bufSize, "\ns%d : : couldn't place an old chain even after reshuffling", dcId);
					printBufToLog ();
					snprintf (buf, bufSize, "\ncpuCapacity=%d chain required cpu=%d", cpuCapacity, chainPtr->mu_u_at_lvl(lvl));
					printBufToLog ();
					printStateAndEndSim  ();
				}
			}
			else { // haven't reshuffled yet --> reshuffle				
				if (MyConfig::mode==Sync) {
					if (MyConfig::LOG_LVL>=DETAILED_LOG) {
						snprintf (buf, bufSize, "\n************** s%d : initiating a reshuffle at lvl %d", dcId, lvl);
						printBufToLog();
					}
					return (MyConfig::useFullResh)? simController->prepareFullReshSync () : prepareReshSync ();
				}
				else {
					return initReshAsync ();
				}
			}
		}
	}

	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : finished BU-f.", dcId);
		printBufToLog ();
		print (false, false, true, false);
	}

  if (isRoot) { 
  	if (!(notAssigned.empty())) {
  		error ("notAssigned isn't empty after running BU on the root");
  	}
  }
  else {
  	return genNsndBottomUpFmodePktAsync ();
  }
}

/************************************************************************************************************************************************
Running the PU alg' at "feasibility" mode
*************************************************************************************************************************************************/
void Datacenter::pushUpFMode ()
{
	error ("sorry, pushUp in Feasibility mode isn't coded yet");
}

/************************************************************************************************************************************************
Running the BU alg'. 
Assume that this->notAssigned and this->pushUpList already contain the relevant chains, and are sorted.
*************************************************************************************************************************************************/
void Datacenter::bottomUp ()
{

	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : beginning BU. notAssigned=", dcId);
		printBufToLog ();
		MyConfig::printToLog (notAssigned);
		MyConfig::printToLog (" pushUpList=");
		MyConfig::printToLog (pushUpList, false);	// "false" means that we print only the chainIds; true would print also each chain's current level
	}

	sort (notAssigned.begin(), notAssigned.end(), SortChainsForNotAssignedList());
	for (auto chainPtr=notAssigned.begin(); chainPtr!=notAssigned.end(); ) {
		Cpu_t requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain(*chainPtr); 
		if (availCpu >= requiredCpuToLocallyPlaceThisChain) { // I have enough avail' cpu for this chain --> assign it
				availCpu -= requiredCpuToLocallyPlaceThisChain;				
				if (cannotPlaceThisChainHigher (*chainPtr)) { // Am I the highest delay-feasible DC of this chain?
					placedChains.		  insert  (chainPtr->id);
					ChainsMaster::modifyLvl  (chainPtr->id, lvl); // inform ChainMaster about the chain's place 
				}
				else { // This chain can be placed higher --> potentially-place it, and insert it to the push-up list, indicating me as its current level
					potPlacedChains.insert (chainPtr->id);
					Chain modifiedChain = *chainPtr;
					modifiedChain.curLvl = lvl;
					modifiedChain.potCpu = requiredCpuToLocallyPlaceThisChain; // set the chain's "potCpu" field to the cpu required, if I'll host it
					if (!insertChainToList (pushUpList, modifiedChain)) {
						error ("Error in insertChainToList. See log file for details");
					}
				}
				chainPtr = notAssigned.erase (chainPtr);

		}
		else { 
			if (canPlaceThisChainHigher(*chainPtr)) { // Am I the highest delay-feasible DC of this chain?
				chainPtr++; //No enough availCpu for this chain, but it may be placed above me --> go on to the next notAssigned chain  
				continue;
			}
			
			// Not enough availCpu for this chain, and it cannot be placed higher
			if (reshuffled) {
				if (chainPtr -> isNew()) { // Failed to place a new chain even after resh
					MyConfig::overallNumBlockedUsrs++;
					if (!ChainsMaster::blockChain (chainPtr->id)) {
						error ("s%d tried to block chain %d that wasn't found in ChainsMaster", dcId, chainPtr->id);
					}
					chainPtr = notAssigned.erase (chainPtr); 
				}
				else { // Failed to place an old chain even after resh
					snprintf (buf, bufSize, "\ns%d : : couldn't find a feasible sol' even after reshuffling", dcId);
					printBufToLog ();
					snprintf (buf, bufSize, "\ncpuCapacity=%d chain required cpu=%d", cpuCapacity, chainPtr->mu_u_at_lvl(lvl));
					printBufToLog ();
					printStateAndEndSim  ();
				}
			}
			else { // haven't reshuffled yet --> reshuffle				
				if (MyConfig::mode==Sync) {
					if (MyConfig::LOG_LVL>=DETAILED_LOG) {
						snprintf (buf, bufSize, "\n************** s%d : initiating a reshuffle at lvl %d", dcId, lvl);
						printBufToLog();
					}
					return (MyConfig::useFullResh)? simController->prepareFullReshSync () : prepareReshSync ();
				}
				else {
					return initReshAsync ();
				}
			}
		}
	}

	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : finished BU. notAssigned=", dcId);
		printBufToLog ();
		MyConfig::printToLog (notAssigned);
		print (true, true, true, true);
	}

  if (isRoot) { 
  	if (MyConfig::printBuRes) {
  		MyConfig::printToLog ("\nAfter BU:");
  		simController->printBuCost ();
  		simController->printAllDatacenters (true, false);
  	}
  	if (!(notAssigned.empty())) {
  		error ("notAssigned isn't empty after running BU on the root");
  	}
	  pushUp ();
  }
  else {
  	return (MyConfig::mode==Sync)? genNsndBottomUpPktSync () : genNsndBottomUpPktAsync ();
  }
}

/*************************************************************************************************************************************************
Update ChainMaster about (the IDs of) all the newly placed chains, as indicated in the newlyPlacedChains input parameter
*************************************************************************************************************************************************/
void Datacenter::updatePlacementInfo (unordered_set <ChainId_t> newlyPlacedChains)
{
	if (!ChainsMaster::modifyLvl (newlyPlacedChains, lvl))	{
		error ("error in ChainsMaster::modifyLvl. See .log file for details.");
	}
}

/*************************************************************************************************************************************************
Handle a bottomUP pkt, when running in Async mode.
*************************************************************************************************************************************************/
void Datacenter::handleBottomUpPktAsync ()
{
	rdBottomUpPkt ();
	bottomUp();
}

/*************************************************************************************************************************************************
Handle a bottomUP pkt, when running in Async FMode.
*************************************************************************************************************************************************/
void Datacenter::handleBottomUpPktAsyncFMode ()
{
	rdBottomUpPkt ();
	bottomUpFMode();
	genNsndPushUpPktsToChildren (); // inform children that I haven't pushed-up anything
}

/*************************************************************************************************************************************************
Read a BU pkt, and add the notAssigned chains,to the respective local ("this") data base.
If not in "F mode", add the chains in pushUpVec into pushUpList
*************************************************************************************************************************************************/
void Datacenter::rdBottomUpPkt ()
{

	BottomUpPkt *pkt = (BottomUpPkt*)(curHandledMsg);
	
	// Add each chain stated in the pkt's notAssigned field into its (sorted) place in this->notAssigned()
	for (int i(0); i < (pkt->getNotAssignedArraySize ());i++) {
		notAssigned.push_back (pkt->getNotAssigned(i));
	}
	
	// Add each chain stated in the pkt's pushUpVec field into this->pushUpList
	for (int i(0); i<pkt -> getPushUpVecArraySize (); i++) {
	  if (!insertChainToList (pushUpList, pkt->getPushUpVec(i))) {
			error ("Error in insertChainToList. See log file for details");
		}        
	}
	
	if (MyConfig::LOG_LVL == VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : handling a BU pkt. src=%d. pushUpList=", dcId, ((Datacenter*) curHandledMsg->getSenderModule())->dcId);
		printBufToLog ();
		MyConfig::printToLog (pushUpList, false);
	}
}

/*************************************************************************************************************************************************
Handle a bottomUP pkt, when running in sync' mode.
- add the notAssigned chains, and the pushUpvec chains, to the respective local ("this") databases.
- delete the pkt.
- If already rcvd a bottomUp pkt from all the children, call the sync' mode of the bottom-up (BU) alg'.
*************************************************************************************************************************************************/
void Datacenter::handleBottomUpPktSync ()
{

	if (numBuPktsRcvd==0) { // this is the first BU pkt rcvd from a child at this period
		notAssigned.clear ();
		pushUpList. clear ();
	}
	numBuPktsRcvd++;
	rdBottomUpPkt ();
	
	if (numBuPktsRcvd == numChildren) { // have I already rcvd a bottomUpMsg from each child?
		bottomUp ();
		numBuPktsRcvd = 0;
	}
}

/*************************************************************************************************************************************************
Generate a BottomUpPkt, based on the data currently found in notAssigned and pushUpList, and xmt it to my parent:
- For each chain in this->pushUpList:
	- if the chain can be placed higher, include it in pushUpList to be xmtd to prnt, and remove it from the this->pushUpList.
For each chain in this->notAssigned:
	- insert the chain into the "notAssigned" field in the pkt to be xmtd to prnt, and remove it from this->notAssigned.
*************************************************************************************************************************************************/
void Datacenter::genNsndBottomUpPktSync ()
{
	BottomUpPkt* pkt2snd = new BottomUpPkt;

	pkt2snd -> setNotAssignedArraySize (notAssigned.size());
	for (int i=0; i<notAssigned.size(); i++) {
		pkt2snd->setNotAssigned (i, notAssigned[i]);
	}

	pkt2snd -> setPushUpVecArraySize (pushUpList.size()); // allocate default size of pushUpVec; will shrink it later to the exact required size.
	int idxInPushUpVec = 0;
	for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); chainPtr++) {
		if (cannotPlaceThisChainHigher (*chainPtr)) { // if this chain cannot be placed higher, there's no use to include it in the pushUpVec to be xmtd to prnt
			continue;
		}
		
		// now we know that this chain can be placed higher --> insert it into the pushUpVec to be xmtd to prnt
		pkt2snd->setPushUpVec (idxInPushUpVec++, *chainPtr);
	}
	pkt2snd -> setPushUpVecArraySize (idxInPushUpVec); // adjust the array's size to the real number of chains inserted into it. 

	sndViaQ (0, pkt2snd); //send the bottomUPpkt to my prnt	
	if (!reshuffled) { 
		notAssigned.clear ();
	}
}



/*************************************************************************************************************************************************
Generate a BottomUpPkt in Async f-mode, based on the data currently found in notAssigned, and xmt it to my parent:
For each chain in this->notAssigned:
	- insert the chain into the "notAssigned" field in the pkt to be xmtd to prnt, and remove it from this->notAssigned.
*************************************************************************************************************************************************/
void Datacenter::genNsndBottomUpFmodePktAsync ()
{
	if (!notAssigned.empty()) {
		BottomUpPkt* pkt2snd = new BottomUpPkt;
		pkt2snd -> setNotAssignedArraySize (notAssigned.size());
		for (int i=0; i<notAssigned.size(); i++) {
			pkt2snd->setNotAssigned (i, notAssigned[i]);
		}
		sndViaQ (0, pkt2snd); //send the bottomUPpkt to my prnt
	}	
	notAssigned.clear ();
}

/*************************************************************************************************************************************************
Generate a BottomUpPkt, based on the data currently found in notAssigned and pushUpList, and xmt it to my parent:
- For each chain in this->pushUpList:
	- if the chain can be placed higher, include it in pushUpList to be xmtd to prnt, and remove it from the this->pushUpList.
For each chain in this->notAssigned:
	- insert the chain into the "notAssigned" field in the pkt to be xmtd to prnt, and remove it from this->notAssigned.
*************************************************************************************************************************************************/
void Datacenter::genNsndBottomUpPktAsync ()
{
	BottomUpPkt* pkt2snd = new BottomUpPkt;

	pkt2snd -> setPushUpVecArraySize (pushUpList.size()); // allocate default size of pushUpVec; will shrink it later to the exact required size.
	int idxInPushUpVec = 0;
	for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); ) {
		if (cannotPlaceThisChainHigher (*chainPtr)) { // if this chain cannot be placed higher, there's no use to include it in the pushUpVec to be xmtd to prnt
			if (!ChainsMaster::modifyLvl (chainPtr->id, chainPtr->curLvl)) { // inform ChainMaster about the chain's place 
				error ("error in Datacenter::genNsndBottomUpPktAsync when trying to update about the new placement of chain %d", chainPtr->id);
			}
			chainPtr++;
			continue;
		}
		
		// now we know that this chain can be placed higher --> insert it into the pushUpVec to be xmtd to prnt
		pkt2snd->setPushUpVec (idxInPushUpVec++, *chainPtr);
		chainPtr = pushUpList.erase (chainPtr); //delete the local entry for this chain in pushUpList; once the push-up repy arrives from the prnt, we'll re-insert it
	}
	
	if (idxInPushUpVec>0 || !notAssigned.empty()) { // there's either pushUp, or notAssigned data to send to prnt
		pkt2snd -> setPushUpVecArraySize (idxInPushUpVec); // adjust the array's size to the real number of chains inserted into it. 

		pkt2snd -> setNotAssignedArraySize (notAssigned.size());
		for (int i=0; i<notAssigned.size(); i++) {
			pkt2snd->setNotAssigned (i, notAssigned[i]);
		}

		sndViaQ (0, pkt2snd); //send the bottomUPpkt to my prnt	
	}
	else { // no really data to send; in async mode there's no use to send an empty pkt, so just destroy it
		delete (pkt2snd);
	}

	notAssigned.clear ();
	if (idxInPushUpVec==0) { // I didn't request prnt to push-up any chain. Hence, no need to wait for his reply --> begin pushUp.
		pushUp ();
	}
}

		
/*************************************************************************************************************************************************
 Init an async reshuffle. called upon a failure to place a chain
*************************************************************************************************************************************************/
void Datacenter::initReshAsync ()
{
	reshInitiatorLvl = lvl; // assign my lvl as the lvl of the initiator of this reshuffle
	isInFMode 			 = true;
	scheduleEndFModeEvent ();
	pushDwnReq.clear (); // verify that the list doesn't contain left-overs from previous runs
	pushDwnAck.clear (); // verify that the list doesn't contain left-overs from previous runs
	deficitCpu = 0;
	for (auto chain : notAssigned ) {
		if (cannotPlaceThisChainHigher(chain)) {
			Cpu_t requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain(chain);
			deficitCpu += requiredCpuToLocallyPlaceThisChain;
			chain.curLvl = lvl;
			chain.potCpu = requiredCpuToLocallyPlaceThisChain;
			insertChainToList (pushDwnReq, chain);
		}
	}
	if (!potPlacedChains.empty()) {
		error ("note: initReshAsync was called when potPlacedChains isn't empty");
	} 
	deficitCpu -= availCpu;
	if (deficitCpu <= 0) {
		error ("initReshAsync was called, but deficitCpu=%d", deficitCpu);
	}
	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\n************** s%d : init resh at lvl %d. pushDwnReq=", dcId, lvl);
		printBufToLog();
		MyConfig::printToLog (pushDwnReq);
	}
	reshAsync ();
}

/*************************************************************************************************************************************************
run the async reshuffle algorithm. Called either by initReshAsync upon a failure to place a chain, or by an arrival of reshAsyncPkt
*************************************************************************************************************************************************/
void Datacenter::reshAsync ()
{

	if (availCpu >= deficitCpu) { // Can finish the resh locally, by placing additional chains on me, w/o calling my children
		pushDwn ();
		if (deficitCpu > 0) {
			error ("at this stage, we should have deficitCpu <= 0");
		}
		//now we know that deficitCpu <= 0, so we can finish the reshuflle
		return finReshAsync ();
	}
	// add my potPlacedChains, and then placedChains, to the end of pushDwnReq
	Chain chain;
	for (ChainId_t chainId : potPlacedChains) {
		if (!ChainsMaster::findChain (chainId, chain)) {
			error ("pot-placed chain %d was not found in ChainMaster", chainId);
		}
		chain.curLvl = lvl;
		chain.potCpu = requiredCpuToLocallyPlaceChain (chain);
		insertChainToList (pushDwnReq, chain);
	}
	for (ChainId_t chainId : placedChains) {
		if (!ChainsMaster::findChain (chainId, chain)) {
			error ("pot-placed chain %d was not found in ChainMaster", chainId);
		}
		chain.curLvl = lvl;
		chain.potCpu = requiredCpuToLocallyPlaceChain (chain);
		insertChainToList (pushDwnReq, chain);
	}
	if (MyConfig::LOG_LVL >= DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d in reshAsync. pushDwnReq=", dcId);
		printBufToLog ();
		MyConfig::printToLog (pushDwnReq);
	}

	// Cannot free enough space --> need to call children. Also, add my 
	if (!sndReshAsyncPktToNxtChild ()) { // send a reshAyncPkt to the next relevant child, if exists
		pushDwn(); // no more children to call --> finish the run of the reshuffling alg' in my sub-tree (including myself)
		return finReshAsync ();
	}
}


/*************************************************************************************************************************************************
Check whether there exists (at least one) additional child to which we should send a reshuffle (in async mode) - and if so, send to him.
returns true iff found a relevant child, and sent him a reshAsyncPkt.
*************************************************************************************************************************************************/
bool Datacenter::sndReshAsyncPktToNxtChild ()
{

	if (MyConfig::LOG_LVL >= DETAILED_LOG && !isLeaf) {
		snprintf (buf, bufSize, "\ns%d in sndToNxtchild. nxtChildToSndReshAsync=%d", dcId, nxtChildToSndReshAsync);
		printBufToLog ();
	}
	list<Chain>  pushDwnReqFromChild; 

	while (nxtChildToSndReshAsync < numChildren) {
		for (auto chainPtr=pushDwnReq.begin(); chainPtr!=pushDwnReq.end(); chainPtr++) {	// consider all the chains in pushDwnReq
			if (chainPtr->S_u[lvl-1]==idOfChildren[nxtChildToSndReshAsync])   { /// this chain is associated with (the sub-tree of) this child
				if (!insertChainToList (pushDwnReqFromChild, *chainPtr)) {
					error ("Error in insertChainToList. See log file for details");
				}
			}
		}
		if (pushDwnReqFromChild.empty()) { // no push-down data to send to this child --> skip it
			nxtChildToSndReshAsync++;
			continue;
		}

		// now we know that pushDwnReqFromChild isn't empty
		if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
			MyConfig::printToLog (". pushDwnReq=");
			MyConfig::printToLog (pushDwnReqFromChild);
		}
		ReshAsyncPkt* pkt2snd = new ReshAsyncPkt;

		pkt2snd -> setReshInitiatorLvl (reshInitiatorLvl); 
		pkt2snd -> setDeficitCpu 		(deficitCpu);
		pkt2snd -> setPushDwnVecArraySize (pushDwnReqFromChild.size());
		
		int idxInPushDwnVec = 0;
		for (auto chainPtr=pushDwnReqFromChild.begin(); chainPtr!=pushDwnReqFromChild.end(); ) {	
			pkt2snd->setPushDwnVec (idxInPushDwnVec++, *chainPtr);
			chainPtr = pushDwnReqFromChild.erase (chainPtr);
		}
		sndViaQ (portToChild(nxtChildToSndReshAsync), pkt2snd); //send the pkt to the child
		nxtChildToSndReshAsync++;
		return true; // successfully sent pkt to the next child	
	}
	if (MyConfig::LOG_LVL >= DETAILED_LOG && !isLeaf) {
		snprintf (buf, bufSize, "\ns%d finished sending to all children", dcId);
		printBufToLog ();
	}
	return false; // no additional relevant child to send to
}


/*************************************************************************************************************************************************
Initiate a print of the content of all the datacenters
*************************************************************************************************************************************************/
void Datacenter::PrintAllDatacenters ()
{
	PrintAllDatacentersMsg* msg2snd = new PrintAllDatacentersMsg; 
	sendDirect (msg2snd, simController, "directMsgsPort");
}

/*************************************************************************************************************************************************
Prepare a reshuffle in Sync mode.
*************************************************************************************************************************************************/
void Datacenter::prepareReshSync () 
{
	if (reshuffled) {
		return genNsndBottomUpPktSync ();	
	}
	reshuffled = true;
	MyConfig::lvlOfHighestReshDc = max (MyConfig::lvlOfHighestReshDc, lvl); // If my lvl is highest then the highest lvl reshuffled at this period - update. 

	clrRsrc ();
	for (int child(0); child<numChildren; child++) { // for each child...
		PrepareReshSyncPkt *pkt = new PrepareReshSyncPkt;
		sndViaQ (portToChild(child), pkt); //send the bottomUPpkt to the child
	}
	
	if (isLeaf) {
		simController->prepareReshSync (dcId, leafId);
	}
}

/*************************************************************************************************************************************************
Clear all the resources currently allocated at this datastore:
- Dis-place all the placed and pot-placed chains.
- Clear notAssigned and pushUpList.
- set availCpu to the initial value.
*************************************************************************************************************************************************/
void Datacenter::clrRsrc () 
{
	notAssigned. 			clear ();
	pushUpList.   		clear ();
	placedChains.			clear ();
	potPlacedChains.	clear ();
	availCpu 				 = cpuCapacity;
}



/*************************************************************************************************************************************************
* Cancel previous EndFModeEvent (if exists), and Schedule a new EndFModeEvent for the current sim time + FModePeriod.
*************************************************************************************************************************************************/
void Datacenter::scheduleEndFModeEvent ()
{
  if (endFModeEvent!=nullptr && endFModeEvent->isScheduled()) { // there's currently an active schedule
    cancelAndDelete (endFModeEvent);
		endFModeEvent = new EndFModeMsg ("");
		scheduleAt(simTime() + MyConfig::FModePeriod, endFModeEvent);
	}
}

/*************************************************************************************************************************************************
 * Send the given packet.
 * If the output port is free, xmt the pkt immediately.
 * Else, queue the pkt until the output port is free, and then xmt it.
*************************************************************************************************************************************************/
void Datacenter::sndViaQ (int16_t portNum, cPacket* pkt2snd)
{
  if (endXmtEvents[portNum]!=nullptr && endXmtEvents[portNum]->isScheduled()) { // if output Q is busy
    outputQ[portNum].insert (pkt2snd);
  }
  else {
    xmt (portNum, pkt2snd);
  }
}

/*************************************************************************************************************************************************
 * Xmt the given pkt to the given output port; schedule a self msg for the end of transmission.
*************************************************************************************************************************************************/
void Datacenter::xmt(int16_t portNum, cPacket* pkt2snd)
{
	send(pkt2snd, "port$o", portNum);

  // Schedule an event for the time when last bit will leave the gate.
  endXmtEvents[portNum] = new EndXmtMsg ("");
  endXmtEvents[portNum]->setPortNum (portNum);
  scheduleAt(xmtChnl[portNum]->getTransmissionFinishTime(), endXmtEvents[portNum]);
}

// return true iff the queried chain id is locally placed
bool Datacenter::isPlaced (ChainId_t chainId) 
{
	auto search = placedChains.find (chainId);
	return (search!=placedChains.end()); 	
}

// return true iff the queried chain id is locally placed
bool Datacenter::isPotentiallyPlaced (ChainId_t chainId) 
{
	auto search = potPlacedChains.find (chainId);
	return (search!=potPlacedChains.end()); 	
}

/*************************************************************************************************************************************************
handle a reshAsyncPkt that arrived from a prnt:
- Insert all the chains into pushDwnReq.
- call reshAsync()
*************************************************************************************************************************************************/
void Datacenter::handleReshAsyncPktFromPrnt  ()
{
	scheduleEndFModeEvent (); // Restart the timer of being in F mode 
	ReshAsyncPkt *pkt = (ReshAsyncPkt*)(curHandledMsg);
	DcId_t reshInitiatorLvl = pkt->getReshInitiatorLvl ();
	if (withinAnotherResh(reshInitiatorLvl)) {
		// send the same pkt back to the prnt
		return sndViaQ (portToPrnt, pkt);
	}

	// now we know that we're not within another reshuffle 	
	this->reshInitiatorLvl = reshInitiatorLvl;
	this->deficitCpu = pkt->getDeficitCpu ();
	for (int i(0); i<pkt->getPushDwnVecArraySize(); i++) {
    if (!insertChainToList (pushDwnReq, pkt->getPushDwnVec(i))) {
			error ("Error in insertChainToList. See log file for details");
		}        
	}
	reshAsync ();
}

/*************************************************************************************************************************************************
Handle a reshuffle async pkt, received from a child.
- Read the pkt's fields.
- dis-place every chain that was pushed-down from me, and release the cpu resources (namely, increase this->availCpu) accordingly.
- if the chain was pushed-down from another Dc (above me), insert it into pushDwnList, that I will later send to my prnt.
- call reshAsync to either call the next child / run the push-down locally / return to bottomUp.
*************************************************************************************************************************************************/
void Datacenter::handleReshAsyncPktFromChild ()
{
	ReshAsyncPkt *pkt = (ReshAsyncPkt*)(curHandledMsg);
	if (withinAnotherResh(pkt->getReshInitiatorLvl ())) {
		error ("rcvd from child a reshAsync pkt with reshInitiator=%d while running another resh with reshInitiatorLvl=%d", 
		pkt->getReshInitiatorLvl (), this->reshInitiatorLvl);
	}
	this->deficitCpu = pkt->getDeficitCpu ();
	
	for (int i(0); i<pkt->getPushDwnVecArraySize(); i++) {
		Chain chain = pkt->getPushDwnVec(i);
		if (chain.curLvl >= lvl) { // the chain wasn't pushed down 
			error ("s%d rcvd a reshAsync pkt from child with lvl above child's lvl", dcId);
		}
		// now we know that the chain was pushed-down to a Dc below me
		if (isPotentiallyPlaced (chain.id)) {
			potPlacedChains.erase (chain.id); 
			regainRsrcOfChain (chain); 
			continue; // finished handling this chain --> no need to enter it into pushDwnAck
		}	
		if (isPlaced(chain.id)) { 
			placedChains.erase (chain.id); 
			regainRsrcOfChain (chain); 
			continue; // finished handling this chain --> no need to enter it into pushDwnAck
		}			
		// now we know that the chain was pushed-down from someone else, above me
		insertChainToList (pushDwnAck, chain);
	 }
	reshAsync ();
}

/*************************************************************************************************************************************************
Finish the local run of a reshAsync alg':
- if I'm the initiator, come back to run bottomUp, but in "F" (feasibility) mode.
- Else, send a reshAsyncPkt to prnt.
*************************************************************************************************************************************************/
void Datacenter::finReshAsync ()
{  

	// make all the "pot-placed" chains "placed"
	for (ChainId_t chainId_t : potPlacedChains) {
		placedChains.insert (chainId_t);
	}
	updatePlacementInfo (potPlacedChains);
	potPlacedChains.clear ();
	if (IAmTheReshIniator()) {
		rstReshAsync ();
		bottomUpFMode (); // come back to bottomUp, but in F ("feasibility") mode
	}
	else {
		sndReshAsyncPktToPrnt ();
		rstReshAsync ();
	}
}

/*************************************************************************************************************************************************
Reset the run of an async resh, and prepare for the next run
*************************************************************************************************************************************************/
void Datacenter::rstReshAsync ()
{
	reshInitiatorLvl 			 = UNPLACED_LVL; // reset the initiator of the currently run reshuffling
	nxtChildToSndReshAsync = 0; 					 // begin the passing over children from child 0
	deficitCpu						 = 0;
	pushDwnReq.clear        ();
	pushDwnAck.clear        ();
}

/*************************************************************************************************************************************************
push-down chains from the list pushDwnReq into me. 
Update state vars (availCpu, deficitCpu, placedChains) accordingly.
Return when either availCpu doesn't suffice to place any additional chain, or when deficitCpu <= 0.
*************************************************************************************************************************************************/
void Datacenter::pushDwn ()
{

	if (MyConfig::LOG_LVL >= DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d push dwn", dcId);
		printBufToLog ();
	}

	for (auto chainPtr=pushDwnReq.begin(); chainPtr!=pushDwnReq.end(); chainPtr++) {	// consider all the chains in pushDwnReq
		if (deficitCpu <= 0 || availCpu < MyConfig::minCpuToPlaceAnyChainAtLvl [lvl]) { 
			break;
		}
		if (chainPtr->curLvl < this->lvl) { 
			error ("my pushDwnReq should include only chains with curLvl >= this.lvl");
		}
		
		// If this chain is placed / potPlaced on me, then availCpu was already decreased when it was placed / pot-placed. No need to decrease it again
		if (isPlaced (chainPtr->id) || isPotentiallyPlaced (chainPtr->id)) { 
			continue; 
		}

		// now we know that this chain wasn't previously placed, or pot-placed, on me
		Cpu_t requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain (*chainPtr);
		if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
			MyConfig::printToLog ("pushed-dwn chains: ");
		}

		if (availCpu >= requiredCpuToLocallyPlaceThisChain) {

			if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
				snprintf (buf, bufSize, "c%d from l%d, ", chainPtr->id, chainPtr->curLvl);
				printBufToLog ();
			}
			availCpu -= requiredCpuToLocallyPlaceThisChain;
			placedChains.insert (chainPtr->id); 				
			ChainsMaster::modifyLvl (chainPtr->id, lvl);
			if (chainPtr->curLvl == reshInitiatorLvl) { // Did I push-down this chain from the initiator?
				deficitCpu -= chainPtr->potCpu;
				if (DEBUG_LVL>0 && requiredCpuToPlaceChainAtLvl (*chainPtr, reshInitiatorLvl) != chainPtr->potCpu) {
					error ("s%d c%d chain.potCpu=%d reshInitiatorLvl=%d, requiredCpuToPlaceChainAtLvl=%d", 
									dcId, chainPtr->id, chainPtr->potCpu, reshInitiatorLvl, requiredCpuToPlaceChainAtLvl (*chainPtr, reshInitiatorLvl));
				}
			}
			if (chainPtr->curLvl > this->lvl) { // Did I push-down this chain from the an ancestor of me?
				Chain pushedDwnChain = *chainPtr;
				pushedDwnChain.curLvl = lvl;
				insertChainToList (pushDwnAck, pushedDwnChain);
			}
		}
	}
}


/*************************************************************************************************************************************************
send a reshAsync pkt to prnt.
In particular, the pushDwnVec field in this pkt will contain pushDwnAck, which should contain details about all the pkts that were pushed-down 
into my sub-tree.
After sending the pkt, pushDwnAck is clear.
*************************************************************************************************************************************************/
void Datacenter::sndReshAsyncPktToPrnt ()
{
	if (MyConfig::LOG_LVL >= DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d snding to prnt", dcId);
		printBufToLog ();
		if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
			MyConfig::printToLog (" pushDwnAck=");
			MyConfig::printToLog (pushDwnAck);
		}
	}

	ReshAsyncPkt* pkt2snd = new ReshAsyncPkt;
	pkt2snd -> setReshInitiatorLvl    (reshInitiatorLvl);
	pkt2snd -> setDeficitCpu 		      (deficitCpu);
	pkt2snd -> setPushDwnVecArraySize (pushDwnAck.size());
	int idxInPushDwnVec = 0;
	for (auto chainPtr=pushDwnAck.begin(); chainPtr!=pushDwnAck.end(); chainPtr++) {	
		pkt2snd->setPushDwnVec (idxInPushDwnVec++, *chainPtr);
	}
	sndViaQ (portToPrnt, pkt2snd);
	pushDwnAck.clear ();
}

