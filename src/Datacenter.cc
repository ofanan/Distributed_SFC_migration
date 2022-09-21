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

inline void	Datacenter::printStateAndEndSim () { sndDirectToSimCtrlr (new cMessage ("PrintStateAndEndSimMsg"));}

inline void Datacenter::regainRsrcOfChain (const Chain &chain) {availCpu += chain.mu_u_at_lvl(lvl); }

inline bool Datacenter::withinResh () const {return this->reshInitiatorLvl!=UNPLACED_LVL;}

inline bool Datacenter::withinAnotherResh (const Lvl_t reshInitiatorLvl) const 
{
	return (withinResh() && this->reshInitiatorLvl!=reshInitiatorLvl);
}

inline bool Datacenter::IAmTheReshIniator () const
{
	return (this->reshInitiatorLvl == this->lvl);
}

inline bool Datacenter::wasPushedUp (const Chain &chain) const 
{
	return (chain.curLvl > this->lvl); 
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
	if (endFModeEvent != nullptr) {
	  cancelAndDelete (endFModeEvent);
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
	
	return curHandledMsg->arrivedOn (prntGateId); 
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

		numPorts    = numParents + numChildren;
		isRoot      = (numParents==0);
		isLeaf      = (numChildren==0);
		outputQ.       resize (numPorts);
		xmtChnl.       resize (numPorts); // the xmt chnl towards each neighbor
		endXmtEvents.  resize (numPorts);
		dcIdOfChild.   resize (numChildren);
		
		// Discover the xmt channels to the neighbors, and the neighbors' id's.
		if (isRoot) {
			if (dcId > 0) {
				error ("s%d was characterized as root", dcId);
			}
			for (int portNum (0); portNum < numPorts; portNum++) {
				cGate *outGate    = gate("port$o", portNum);
				xmtChnl[portNum]  = outGate->getTransmissionChannel();
				cModule *nghbr    = outGate->getNextGate()->getOwnerModule();
				dcIdOfChild  [portNum] = DcId_t (nghbr -> par ("dcId"));
			}
		}
		else {
			cGate *outGate    = gate("port$o", 0);
			xmtChnl[0]  			= outGate->getTransmissionChannel();
			cModule *nghbr    = outGate->getNextGate()->getOwnerModule();
			idOfParent = DcId_t (nghbr -> par ("dcId"));

			// ports 1...numChildren are towards the children
			for (int portNum (1); portNum < numPorts; portNum++) {
				outGate    			 = gate("port$o", portNum);
				xmtChnl[portNum] = outGate->getTransmissionChannel();
				nghbr   			   = outGate->getNextGate()->getOwnerModule();
			  dcIdOfChild  [portNum-1] = DcId_t (nghbr -> par ("dcId"));
			}
			prntGateId = gate("port$i", 0)->getId();
		}
		fill(endXmtEvents. begin(), endXmtEvents. end(), nullptr);
		return;
	}
	
	MY_ACCUMULATION_DELAY = ACCUMULATION_DELAY * (1+float(lvl/2));
	// parameters that depend upon MyConfig can be initialized only after stage 0, in which MyConfig is initialized.
	cpuCapacity   = MyConfig::cpuAtLvl[lvl]; 
  availCpu    	= cpuCapacity; // initially, all cpu rsrcs are available (no chain is assigned)
	rstReshAsync ();
	endFModeEvent  = nullptr;
	isInFMode 		 = false;
	isInAccumDelay = false;
}

/*************************************************************************************************************************************************
 * Schedule an error msg and exit.
 * The exit isn't done immediately, for letting other modules print their data before finishing the sim, thus easing the debugging.
*************************************************************************************************************************************************/
void Datacenter::scheduleErrMsgAndExit ()
{
	scheduleAt (simTime() + MY_ACCUMULATION_DELAY, new cMessage ("errorMsg")); 
}

/*************************************************************************************************************************************************
 * Sanity-check for the DC's state at the end of a time period.
*************************************************************************************************************************************************/
void Datacenter::checkEndTimePeriod ()
{
	Enter_Method ("Datacenter::checkEndTimePeriod ()");
	if (!potPlacedChains.empty()) {
		error ("t=%f. s%d : potPlacedChains is not empty\n", MyConfig::traceTime, dcId);
	}
	if (!pushUpList.empty()) {
		sprintf (buf, "\nt=%f. s%d : error: pushUpList is not empty. PUL=\n", MyConfig::traceTime, dcId);
		printBufToLog ();
		MyConfig::printToLog (pushUpList);
		error ("t=%f. s%d : pushUpList is not empty\n", MyConfig::traceTime, dcId);
	}
	for (auto item : endXmtEvents) {
		if (item!=nullptr) {
			error ("t=%f s%d still have xmt event in end of time period", MyConfig::traceTime, dcId);
		}
	}
	for (int portNum(0); portNum<numPorts; portNum++) { 
		if (!outputQ[portNum].isEmpty() 	 	) {
			error ("t=%f s%d : outputQ %d isn't empty in the end of a time period", MyConfig::traceTime, dcId, portNum);
		}
	}
	if (MyConfig::mode==Sync) {
		return; // the next checks are for Async mode only
	}
	if (!notAssigned.empty()) {
		error ("t=%f. s%d : notAssigned is not empty\n", MyConfig::traceTime, dcId);
	}
	if (!pushDwnReq.empty()) {
		error ("t=%f. s%d : pushDwnReq is not empty\n", MyConfig::traceTime, dcId);
	}
	if (!pushDwnAck.empty()) {
		error ("t=%f. %d : pushDwnAck is not empty\n", MyConfig::traceTime, dcId);
	}
}

/*************************************************************************************************************************************************
 * Print to the log file data about chains on this DC. 
 * Inputs: 
 * - printChainIds: when true, print in a format similar to that used in the centralized alg'.
 * - printPotPlaced: when true and there're potPlacedChains to print - print them.
 * - printPushUpList: when true and pushUpList isn't empty - print them.
 * - beginWithNewLine: when true, the print begins in a new line, and with the dcId.
*************************************************************************************************************************************************/
void Datacenter::print (bool printPotPlaced, bool printPushUpList, bool printChainIds, bool beginWithNewLine)
{

	Enter_Method ("Datacenter::print (bool printPotPlaced, bool printPushUpList, bool printChainIds, bool beginWithNewLine)");

	if (placedChains.empty() && (!printPotPlaced || potPlacedChains.empty()) && (!printPushUpList || pushUpList.empty())) {
//		sprintf (buf, "\ns%d : empty", dcId);
//		printBufToLog ();
		return;
	}
	if (beginWithNewLine) {
		snprintf (buf, bufSize, "\ns%d : Rcs=%d, a=%d, used cpu=%d, num_of_placed_chains=%d", 
														dcId, cpuCapacity, availCpu, cpuCapacity-availCpu, int(placedChains.size()) );
	}
	else {
		snprintf (buf, bufSize, " Rcs=%d, a=%d, used cpu=%d, num_of_placed_chains=%d", cpuCapacity, availCpu, cpuCapacity-availCpu, int(placedChains.size()) );
	}
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
calculate the len of a pkt, given the # of Rt and NonRt chains in it
*************************************************************************************************************************************************/
inline int Datacenter::bitLengthOfPkt (const int numRtChains, const int &numNonRtChains)
{
	return MyConfig::sizeofRtChain * numRtChains + MyConfig::sizeofNonRtChain * numNonRtChains;
}


/*************************************************************************************************************************************************
increase the count of # of chains info that I will send in the next pkt
*************************************************************************************************************************************************/
void Datacenter::incChainsInPktCnt (Chain &chain, int &numRtChains, int &numNonRtChains)
{
	if (chain.isRtChain) {
		numRtChains++;
	}
	else {
		numNonRtChains++;
	}
}

///*************************************************************************************************************************************************
//increase the count of # of chains info that I will send to my prnt  in the next pkt
//*************************************************************************************************************************************************/
//void Datacenter::incChainsUpCnt ()
//{
//}

///*************************************************************************************************************************************************
//increase the count of # of chains info that I will send to my prnt  in the next pkt
//*************************************************************************************************************************************************/
//void Datacenter::incChainsDwnCnt ()
//{
//}

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


/*************************************************************************************************************************************************
* handleMessage
*************************************************************************************************************************************************/
void Datacenter::handleMessage (cMessage *msg)
{

  curHandledMsg = msg;
	if (MyConfig::discardAllMsgs) {
		delete curHandledMsg;
		return;
	}
  else if (dynamic_cast<EndXmtMsg*>(curHandledMsg) != nullptr) {
  	handleEndXmtMsg ();
  }
  else if (msg->isSelfMessage() && strcmp (msg->getName(), "endFModeEvent")==0) { 
  	if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
  		snprintf (buf, bufSize, "\ns%d : exiting F mode", dcId);
  		printBufToLog ();
  	}
  	isInFMode     = false;
  	endFModeEvent = nullptr; 
  }
  else if (msg->isSelfMessage() && strcmp (msg->getName(), "initReshAsync")==0) {
  	isInAccumDelay = false;
  	initReshAsync (); 
  }
  else if (msg->isSelfMessage() && strcmp (msg->getName(), "bottomUp")==0) {
	  isInBuAccumDelay = false;
  	bottomUp();
	}
  else if (strcmp (msg->getName(), "prepareReshSyncMsg")==0) {
    if (MyConfig::mode==Sync) { 
    	prepareReshSync ();
    }
    else {
    	error ("rcvd a prepareReshSyncMsg while being in Async mode");
    }
  }
  else if (strcmp (msg->getName(), "errorMsg")==0) {
 		printBufToLog ();
 		if (MyConfig::DEBUG_LVL >=0) {
	 		error (buf);
	 	}
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
    error ("t=%f : s%d : rcvd a pkt of an unknown type", MyConfig::traceTime, dcId);
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
				if (chain.S_u.size()==0) {
					error ("rlzRsrc () : placed chain %d has S_u_len=0%", chainId);
				}
				else {
					error ("rlzRsrc () : placed chain %d was not found in ChainMaster", chainId);
				}
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
				if (chain.S_u.size()==0) {
					error ("rlzRsrc () : placed chain %d has S_u_len=0%", chainId);
				}
				else {
					error ("rlzRsrc () : placed chain %d was not found in ChainMaster", chainId);
				}
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
		error ("t=%f, Non-leaf s%d : was called by initBottomUp", MyConfig::traceTime, dcId);
	}
	pushUpList.				clear ();	
	potPlacedChains.  clear ();
	notAssigned = vecOfChainsThatJoined;

	if (MyConfig::DEBUG_LVL>0) {
		for (auto chain : notAssigned) {
		  if (chain.S_u.size() ==0) {
		    error ("s%d : encountered c%d with S_u_len=0", dcId, chain.id);
		  }
		}
	}

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
		
		if (!checkNinsertChainToList (pushUpList, pkt->getPushUpVec (i))) {
			 error ("t%f. Error in insertChainToList. See log file for details", MyConfig::traceTime);
		}
	}
	
	if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
		sprintf (buf, "\ns%d : rcvd PU pkt. PUL=", dcId);
		printBufToLog ();
		MyConfig::printToLog (pushUpList);
		MyConfig::printToLog (" placed=");
		MyConfig::printToLog (placedChains);
		MyConfig::printToLog (" potPlaced=");
		MyConfig::printToLog (potPlacedChains);
		sprintf (buf, " my lvl=%d", this->lvl);
		printBufToLog ();
	}

	regainRsrcOfpushedUpChains ();
	if (isInAccumDelay) {
		if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
			snprintf (buf, bufSize, "\ns%d : rding PU pkt during accum delay", dcId);
			printBufToLog ();
		}
		return genNsndPushUpPktsToChildren();
	}
	if (isInFMode) {
		genNsndPushUpPktsToChildren();
	}
	else {
		pushUp();
	}
}


/*************************************************************************************************************************************************
 Find all chains that were pushed-up from me, and regain resources for them.
*************************************************************************************************************************************************/
void Datacenter::regainRsrcOfpushedUpChains ()
{
	for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); ) { // for each chain in pushUpList
		if (isPotPlaced(chainPtr->id)) { 
			potPlacedChains.erase (chainPtr->id);
			if (wasPushedUp (*chainPtr)) {
				regainRsrcOfChain (*chainPtr);
			}
			else {
				placedChains.insert (chainPtr->id); 			
				if (!ChainsMaster::modifyLvl  (chainPtr->id, lvl)){ // inform ChainMaster about the chain's place 
					error ("Datacenter::RegainRsrcOfPushedUpChains failed to update the lvl of c%d", chainPtr->id);
				}
			}
			potPlacedChains.erase (chainPtr->id);
			chainPtr = pushUpList.erase (chainPtr); // finished handling this chain --> remove it from the pushUpList, and go on to the next chain
			continue;
		}
		if (isPlaced (chainPtr->id)) { 
			if (MyConfig::DEBUG_LVL>0 && MyConfig::mode==Sync) {
				error ("c%d that I already placed appears in a PUL that I recevied. This is illegal in Sync mode", chainPtr->id);
			}
		
			if (wasPushedUp (*chainPtr)) { //This chain was placed on me, but later it was pushed-up by an ancestor --> displace the chain
				regainRsrcOfChain (*chainPtr);
				placedChains.erase (chainPtr -> id);
				
				// We don't inform ChainsMaster, as the Dc that pushed-up that chain should have already informed ChainsMaster.
				chainPtr = pushUpList.erase (chainPtr); // finished handling this chain --> remove it from the pushUpList, and go on to the next chain
				continue;
			}
		}
		if (isLeaf) { // no children to which I'll have to send info about this chain --> erase it from my PUL.
			chainPtr = pushUpList.erase (chainPtr); 
			continue;
		}
		chainPtr++;
	}
}

/*************************************************************************************************************************************************
Run the PU alg' (either in Sync / Async) mode. 
Assume that this->pushUpList already contains the relevant chains.
*************************************************************************************************************************************************/
void Datacenter::pushUp ()
{

	if (MyConfig::LOG_LVL>=TLAT_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : beginning PU. pushUpList=", dcId);
		printBufToLog ();
		MyConfig::printToLog (pushUpList, false);
	}
	reshuffled = false;
	
	// Try to push-up chains of my descendants
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
					chainPtr++;
					continue;
		}
		else { // the chain is currently placed on a descendant, and I have enough place for this chain-->push up this chain to me
			availCpu 						-= requiredCpuToLocallyPlaceThisChain;
			Chain pushedUpChain  = *chainPtr; // construct a new chain to insert to placedChains, because it's forbidden to modify the chain in pushUpList
			pushedUpChain.curLvl = lvl;
			chainPtr 						 = pushUpList.erase (chainPtr); // remove the pushed-up chain from the list of potentially pushed-up chains; to be replaced by a modified chain
			placedChains.				 insert (pushedUpChain.id);
			if (!ChainsMaster::modifyLvl  (pushedUpChain.id, lvl)){ // inform ChainMaster about the chain's place 
				error ("Datacenter::bottomUpFMode failed to update the lvl of c%d", chainPtr->id);
			}
			pushedUpChains.insert (pushedUpChains.begin(), pushedUpChain);
		}
	}
	
	for (auto chainPtr=pushedUpChains.begin(); chainPtr!=pushedUpChains.end(); chainPtr++) {
		if (MyConfig::DEBUG_LVL>0 && chainPtr->S_u.size() ==0) {
			error ("s%d : encountered pushedUpChain c%d with S_u_len=0", dcId, chainPtr->id);
		}
		checkNinsertChainToList (pushUpList, *chainPtr);
	}
	
	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : finished PU.", dcId);
		printBufToLog ();
		print (false, true, true, false);
	}

	if (isLeaf && MyConfig::mode == Sync) {
		return simController->finishedAlg (dcId, leafId);
	}

	genNsndPushUpPktsToChildren ();
	
}

/*************************************************************************************************************************************************
Generate pushUpPkts, based on the data currently found in pushUpList, and xmt these pkts to all the children
*************************************************************************************************************************************************/
void Datacenter::genNsndPushUpPktsToChildren ()
{
    if (isLeaf) {
        return;
    }
	PushUpPkt* pkt;	 // the packet to be sent 
	for (Lvl_t child(0); child<numChildren; child++) { // for each child...
		pkt = new PushUpPkt;
		int numRtChains=0, numNonRtChains=0;
		pkt->setPushUpVecArraySize (pushUpList.size ()); // default size of pushUpVec, for case that all chains in pushUpList belong to this child; will later shrink pushUpVec otherwise 
		int idxInPushUpVec = 0;
		for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); ) {	// consider all the chains in pushUpList
			if (chainPtr->S_u[lvl-1]==dcIdOfChild[child])   { /// this chain is associated with (the sub-tree of) this child
				pkt->setPushUpVec (idxInPushUpVec++, *chainPtr);
				incChainsInPktCnt (*chainPtr, numRtChains, numNonRtChains);
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
			if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
				sprintf (buf, "\n s%d : snding PU pkt to child %d. simTime=%f, PUL=", dcId, dcIdOfChild[child], simTime().dbl());
				printBufToLog ();
				for (int i(0); i<idxInPushUpVec; i++) {
					Chain chain2print = pkt-> getPushUpVec (i);
					sprintf (buf, ",c%d,l%d ", chain2print.id, chain2print.curLvl);
					printBufToLog ();				
				}
			}
		}
		else {
			delete (pkt);
		}
	}
	if (MyConfig::DEBUG_LVL>0 && MyConfig::mode==Sync && !pushUpList.empty()) {
		error ("pushUpList is not empty after sending PU pkts to all children");
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
		if (availCpu >= requiredCpuToLocallyPlaceThisChain) { // I have enough avail' cpu for this chain --> place it
				availCpu -= requiredCpuToLocallyPlaceThisChain;				
				placedChains.		  insert  (chainPtr->id);
				if (!ChainsMaster::modifyLvl  (chainPtr->id, lvl)){ // inform ChainMaster about the chain's place 
					error ("Datacenter::bottomUpFMode failed to update the lvl of c%d", chainPtr->id);
				}
				chainPtr = notAssigned.erase (chainPtr);
		}
		else { 
			if (canPlaceThisChainHigher(*chainPtr)) { // If I'm not the highest delay-feasible DC of this chain...
				chainPtr++; // go on to the next notAssigned chain  
				continue;
			}
			
			// Not enough availCpu for this chain, and it cannot be placed higher
			if (withinResh ()) { // within (or just finished) a reshuffle --> don't initiate a new reshuffle, even upon a failure to place a chain
				if (chainPtr -> isNew()) { // Failed to place a new chain even after resh
					if (!ChainsMaster::blockChain (chainPtr->id)) {
						error ("s%d tried to block chain %d that wasn't found in ChainsMaster", dcId, chainPtr->id);
					}
//					if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) { //$$
						sprintf (buf, "\ns%d : blocked chain %d", dcId, chainPtr->id);
						printBufToLog ();
//					} //$$
					chainPtr = notAssigned.erase (chainPtr); 
				}
				else { // Failed to place an old chain even after resh
					return failedToPlaceOldChain (chainPtr->id);
				}
			}
			else { // not within a reshuffle
				this->reshInitiatorLvl = this->lvl; // assign my lvl as the lvl of the initiator of this reshuffle
				isInFMode 			 			 = true;      // set myself to "F" mode
				isInAccumDelay				 = true;
				if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
					sprintf (buf, "\ns%d : schedule initReshAsync", dcId);
					printBufToLog ();
				}	
				return scheduleAt (simTime() + MY_ACCUMULATION_DELAY, new cMessage ("initReshAsync")); //schedule a reshuffle
			}
		} // end case of not enough avail capacity
	} // end for
	
	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : traceTime=%f. finished BU-f.", dcId, MyConfig::traceTime);
		printBufToLog ();
		print (false, true, true, false);
	}

	genNsndPushUpPktsToChildren (); // if there're any "left-over" push-up requests from children, just send them "as is" to the caller.
  if (isRoot) {
  	if (notAssigned.empty()) {
  		return; // I'm the root and no PU happens in F-mode, so there's nothing to do anymore
  	}
  	else {
  		error ("notAssigned isn't empty after running BU on the root");
	  }
  }
  else {
  	return genNsndBottomUpFmodePktAsync ();
  }
}

/************************************************************************************************************************************************
Handle a failure to place an old (exiting) chain
*************************************************************************************************************************************************/
void Datacenter::failedToPlaceOldChain (ChainId_t chainId)
{
	if (MyConfig::LOG_LVL>0) {
		sprintf (buf, "\ntraceTime=%.3f, s%d : error : failed to place old chain %d even after reshuffling. notAssigned=\n", MyConfig::traceTime, dcId, chainId);
		printBufToLog ();
	}	
	simController-> handleAlgFailure ();
}

/************************************************************************************************************************************************
Run the BU alg', assuming that this->notAssigned and this->pushUpList already contain the relevant chains, and are sorted.
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
					if (!ChainsMaster::modifyLvl  (chainPtr->id, lvl)){ // inform ChainMaster about the chain's place 
						error ("Datacenter::bottomUpFMode failed to update the lvl of c%d", chainPtr->id);
					}
				}
				else { // This chain can be placed higher --> potentially-place it, and insert it to the push-up list, indicating me as its current level
					potPlacedChains.insert (chainPtr->id);
					Chain modifiedChain = *chainPtr;
					modifiedChain.curLvl = lvl;
					modifiedChain.potCpu = requiredCpuToLocallyPlaceThisChain; // set the chain's "potCpu" field to the cpu required, if I'll host it
					if (MyConfig::DEBUG_LVL>0 && modifiedChain.S_u.size() ==0) {
						error ("s%d : encountered modifiedChain c%d with S_u_len=0", dcId, modifiedChain.id);
					}
					if (!checkNinsertChainToList (pushUpList, modifiedChain)) {
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
			if (MyConfig::mode==Sync) {
				if (reshuffled) {
					if (chainPtr -> isNew()) { // Failed to place a new chain even after resh
						MyConfig::overallNumBlockedUsrs++;
						if (!ChainsMaster::blockChain (chainPtr->id)) {
							error ("s%d tried to block chain %d that wasn't found in ChainsMaster", dcId, chainPtr->id);
						}
						sprintf (buf, "BU Sync blocked a chain");
						if (MyConfig::LOG_LVL>0) {
							printBufToLog ();
							cout << buf << endl;
						}
						return simController-> handleAlgFailure ();
					if (MyConfig::LOG_LVL>=DETAILED_LOG) { 
						sprintf (buf, "\ns%d : blocked chain %d", dcId, chainPtr->id);
						printBufToLog ();
					} 
						chainPtr = notAssigned.erase (chainPtr); 
					}
					else { // Failed to place an old chain even after resh
						return failedToPlaceOldChain (chainPtr->id);
					}
				}
				else { // I'm in sync mode, and haven't reshuffled yet at this cycle
					if (MyConfig::LOG_LVL>=DETAILED_LOG) {
						snprintf (buf, bufSize, "\n************** s%d : initiating a reshuffle at lvl %d", dcId, lvl);
						printBufToLog();
					}
					return (MyConfig::useFullResh)? simController->prepareFullReshSync () : prepareReshSync ();	// perform full / partial resh, 
				}
			}
			else { // Async mode
				this->reshInitiatorLvl = this->lvl; // assign my lvl as the lvl of the initiator of this reshuffle
				isInFMode 			 			 = true;      // set myself to "F" mode
				isInAccumDelay				 = true;
				if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
					sprintf (buf, "\ns%d : schedule initReshAsync", dcId);
					printBufToLog ();
				}	
				return scheduleAt (simTime() + MY_ACCUMULATION_DELAY, new cMessage ("initReshAsync")); //reshuffle, after clearance delay (for letting other children call me)
			}
		} // end case of not enough avail capacity
	} // end "for each chain ...loop"

	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : finished BU. notAssigned=", dcId);
		printBufToLog ();
		MyConfig::printToLog (notAssigned);
		print (true, true, true, false);
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
  	return (MyConfig::mode==Sync)? genNsndBottomUpPktSync () : genNsndBottomUpPktAsync (); // if needed, send my parent a BU pkt
  }
}

/*************************************************************************************************************************************************
Handle a bottomUP pkt, when running in Async mode:
- Write log messages, if needed.
- Read the pkt's content into this->notAssigned, and this->pushUpList.
- Call bottomUp () for running the BU algorithm.
*************************************************************************************************************************************************/
void Datacenter::handleBottomUpPktAsync ()
{
	if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : handling a BU pkt. src=%d. notAssigned=", dcId, ((Datacenter*) curHandledMsg->getSenderModule())->dcId);
		printBufToLog ();
		MyConfig::printToLog (notAssigned);
		MyConfig::printToLog (", pushUpList=");
		MyConfig::printToLog (pushUpList, false);
	}
	rdBottomUpPkt ();
	if (isInBuAccumDelay) {
		return;
	}	
	isInBuAccumDelay = true;
	scheduleAt (simTime() + MY_ACCUMULATION_DELAY, new cMessage ("bottomUp")); //schedule a run of bottomUp
}

/*************************************************************************************************************************************************
Handle a bottomUP pkt, when running in Async F-mode.
- If the pkt's pushUpVec field isn't empty, reply the sender with a PU pkt having the same pushUpVec.
- Read notAssigned field from the pkt, and call bottomUpFMode() to process it.
*************************************************************************************************************************************************/
void Datacenter::handleBottomUpPktAsyncFMode ()
{	

	if (isInAccumDelay) {
		if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
			snprintf (buf, bufSize, "\ns%d : rding BU pkt from s%d during accum delay", dcId, ((Datacenter*) curHandledMsg->getSenderModule())->dcId);
			printBufToLog ();
		}
		return rdBottomUpPkt ();
	}

	if (withinResh ()) {
		if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
			snprintf (buf, bufSize, "\ns%d : handling a BU pkt in f mode from s%d. I'm within resh", dcId, ((Datacenter*) curHandledMsg->getSenderModule())->dcId);
			printBufToLog ();
		}
		return rdBottomUpPkt ();
	}

	DcId_t sndrDcId = ((Datacenter*) curHandledMsg->getSenderModule())->dcId;

	BottomUpPkt *arrivedPkt = (BottomUpPkt*)(curHandledMsg);
	int pushUpVecArraySize = arrivedPkt->getPushUpVecArraySize();
	if (pushUpVecArraySize>0) {

		Lvl_t child;
		for (child=0; child < numChildren; child++) {
			if (dcIdOfChild[child] == sndrDcId) {
				break;
			}
		}
		if (child==numChildren) {
			error ("t=%f s%d couldn't extract the sender's id", MyConfig::traceTime, dcId);
		}

		PushUpPkt* pkt2snd = new PushUpPkt;
		pkt2snd->setPushUpVecArraySize (pushUpVecArraySize); 
		for (int i(0); i<pushUpVecArraySize; i++) {
			pkt2snd->setPushUpVec (i, arrivedPkt->getPushUpVec (i));
		}
		sndViaQ (portToChild(child), pkt2snd); //send the pkt to the child
	}

	// Add each chain stated in the pkt's notAssigned field into its (sorted) place in this->notAssigned()
	for (int i(0); i < (arrivedPkt->getNotAssignedArraySize ());i++) {
		notAssigned.push_back (arrivedPkt->getNotAssigned(i));
	}
		
	if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : handling a BU pkt in f mode from s%d. not within resh. notAssigned=", dcId, ((Datacenter*) curHandledMsg->getSenderModule())->dcId);
		printBufToLog ();
		MyConfig::printToLog (notAssigned);
		MyConfig::printToLog (", pushUpList=");
		MyConfig::printToLog (pushUpList, false);
	}
	bottomUpFMode();
}

/*************************************************************************************************************************************************
Read a BU pkt, and add the notAssigned chains, and the pushUpVec, to the respective local ("this") data base.
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
	  if (!checkNinsertChainToList (pushUpList, pkt->getPushUpVec(i))) {
			error ("Error in insertChainToList. See log file for details");
		}
	}
	
	if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : finished reading a BU pkt from s%d. notAssigned=", dcId, ((Datacenter*) curHandledMsg->getSenderModule())->dcId);
		printBufToLog ();
		MyConfig::printToLog (notAssigned);
		MyConfig::printToLog (" pushUpList=");
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
	int numRtChains=0, numNonRtChains=0;
	for (int i=0; i<notAssigned.size(); i++) {
		pkt2snd->setNotAssigned (i, notAssigned[i]);
		incChainsInPktCnt (notAssigned[i], numRtChains, numNonRtChains);
	}

	pkt2snd -> setPushUpVecArraySize (pushUpList.size()); // allocate default size of pushUpVec; will shrink it later to the exact required size.
	int idxInPushUpVec = 0;
	for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); chainPtr++) {
		if (cannotPlaceThisChainHigher (*chainPtr)) { // if this chain cannot be placed higher, there's no use to include it in the pushUpVec to be xmtd to prnt
			continue;
		}
		
		// now we know that this chain can be placed higher --> insert it into the pushUpVec to be xmtd to prnt
		pkt2snd->setPushUpVec (idxInPushUpVec++, *chainPtr);
		incChainsInPktCnt (*chainPtr, numRtChains, numNonRtChains);
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
		int numRtChains=0, numNonRtChains=0; 
		pkt2snd -> setNotAssignedArraySize (notAssigned.size());
		for (int i=0; i<notAssigned.size(); i++) {
			pkt2snd->setNotAssigned (i, notAssigned[i]);
			incChainsInPktCnt (notAssigned[i], numRtChains, numNonRtChains);
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
If, after this process, there's no data to send to my parent, return without doing anything.
(else), xmt this generated BU pkt to my prnt.
*************************************************************************************************************************************************/
void Datacenter::genNsndBottomUpPktAsync ()
{
	BottomUpPkt* pkt2snd = new BottomUpPkt;

	pkt2snd -> setPushUpVecArraySize (pushUpList.size()); // allocate default size of pushUpVec; will shrink it later to the exact required size.
	int numRtChains=0, numNonRtChains=0;
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
		incChainsInPktCnt (*chainPtr, numRtChains, numNonRtChains);
		chainPtr = pushUpList.erase (chainPtr); //erase the local entry for this chain in pushUpList; once the push-up repy arrives from the prnt, we'll re-insert it
	}
	
	if (idxInPushUpVec>0 || !notAssigned.empty()) { // there's either pushUp, or notAssigned data to send to prnt
		pkt2snd -> setPushUpVecArraySize (idxInPushUpVec); // adjust the array's size to the real number of chains inserted into it. 

		pkt2snd -> setNotAssignedArraySize (notAssigned.size());
		for (int i=0; i<notAssigned.size(); i++) {
			pkt2snd->setNotAssigned (i, notAssigned[i]);
			incChainsInPktCnt (notAssigned[i], numRtChains, numNonRtChains);
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
	this->reshInitiatorLvl = this->lvl; // assign my lvl as the lvl of the initiator of this reshuffle
	MyConfig::lvlOfHighestReshDc = max (MyConfig::lvlOfHighestReshDc, lvl); // If my lvl is higher then the highest lvl reshuffled at this period - update. 
	isInFMode 			 = true;
	scheduleEndFModeEvent ();
	pushDwnReq.clear (); // verify that the list doesn't contain left-overs from previous runs
	pushDwnAck.clear (); // verify that the list doesn't contain left-overs from previous runs
	deficitCpu = 0;
	
	// init pushDwnReq (list of chains I will request to to push down), and calc deficitCpu.
	for (auto chain : notAssigned ) {
		if (cannotPlaceThisChainHigher(chain)) {
			Cpu_t requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain(chain);
			deficitCpu += requiredCpuToLocallyPlaceThisChain;
			Chain chain2pushDwn = chain;
			chain2pushDwn.curLvl = lvl;
			chain2pushDwn.potCpu = requiredCpuToLocallyPlaceThisChain;
			checkNinsertChainToList (pushDwnReq, chain2pushDwn);
		}
	}
	deficitCpu -= availCpu;
	if (deficitCpu <= 0) { // there's no deficit anymore
		return bottomUpFMode ();
	}
	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : *** simT=%.6f init resh at lvl %d. pushDwnReq=", dcId, simTime().dbl(), lvl);
		printBufToLog();
		MyConfig::printToLog (pushDwnReq);
	}
	insertMyAssignedChainsIntoPushDwnReq ();
	reshAsync ();
}

/*************************************************************************************************************************************************
insert a chain to a list.
If the chain (recognized equivocally by its id) is already found in the list, the old occurance in the list is deleted.
If DEBUG_LVL requests it, then before inserting the chain to the list, perform a sanity check to the chain.
*************************************************************************************************************************************************/
bool Datacenter::checkNinsertChainToList (list <Chain> &listOfChains, Chain &chain)
{
  if (MyConfig::DEBUG_LVL>0 && chain.S_u.size()==0) {
    error ("s%d : encountered c%d with S_u_len==0", dcId, chain.id);
  }
	return insertChainToList (listOfChains, chain); 
}
 
/*************************************************************************************************************************************************
run the async reshuffle algorithm. Called either by initReshAsync upon a failure to place a chain, or by an arrival of reshAsyncPkt
*************************************************************************************************************************************************/
void Datacenter::reshAsync ()
{

	isInFMode = true;
	// Check first if I can solve the deficit prob' locally, by pushing down to me, w/o calling children
	bool canFinReshLocally = true;
	Cpu_t deficitCpuThatCanBeResolvedLocally = 0;
	for (auto chainPtr=pushDwnReq.begin(); chainPtr!=pushDwnReq.end(); chainPtr++) {
		if (chainPtr->curLvl==this->reshInitiatorLvl) { // can pushed-down a chain from the resh initiator
				deficitCpuThatCanBeResolvedLocally += chainPtr->potCpu;
		}
		if (deficitCpuThatCanBeResolvedLocally > availCpu) { // don't have enough availCpu to resolve the prob' by placing chains locally
			canFinReshLocally = false;
			break;
		}
	}
	if (canFinReshLocally || isLeaf) {
	  if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
	  	sprintf (buf, "\ns%d : in finReshAsync locally. Placed=", dcId);
	  	printBufToLog ();
	  	MyConfig::printToLog(placedChains);
	  	MyConfig::printToLog ("potPlaced=");
	  	MyConfig::printToLog(potPlacedChains);
	  	MyConfig::printToLog ("pushDwnReq=");
	  	MyConfig::printToLog(pushDwnReq);
	  }
		pushDwn ();
		return finReshAsync ();
	}

	// Cannot free enough space alone --> need to call additional child. 
	if (MyConfig::LOG_LVL >= DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : in reshAsync. pushDwnReq=", dcId);
		printBufToLog ();
		MyConfig::printToLog (pushDwnReq);
	}

	if (!sndReshAsyncPktToNxtChild ()) { // send a reshAyncPkt to the next relevant child, if exists
		pushDwn(); // no more children to call --> finish the run of the reshuffling alg' in my sub-tree (including myself)
		return finReshAsync ();
	}
}

/*************************************************************************************************************************************************
* add my potPlacedChains, and then placedChains, to the end of pushDwnReq
*************************************************************************************************************************************************/
void Datacenter::insertMyAssignedChainsIntoPushDwnReq ()
{
	
	Chain chain;
	for (ChainId_t chainId : potPlacedChains) {
		if (!ChainsMaster::findChain (chainId, chain)) {
			if (chain.S_u.size()==0) {
				error ("traceTime=%.0ff : insertMyAssignedChainsIntoPushDwnReq () : potPlaced chain %d has S_u_len=0%", MyConfig::traceTime, chainId);
			}
			else {
				error ("traceTime=%.0f : insertMyAssignedChainsIntoPushDwnReq () : potPlaced chain %d was not found in ChainMaster", MyConfig::traceTime, chainId);
			}
		}
		Chain chain2insert = chain;
		chain2insert.curLvl = lvl;
		chain2insert.potCpu = requiredCpuToLocallyPlaceChain (chain2insert);
		checkNinsertChainToList (pushDwnReq, chain2insert);
	}
	for (ChainId_t chainId : placedChains) {
		if (!ChainsMaster::findChain (chainId, chain)) {
			if (chain.S_u.size()==0) {
				error ("traceTime=%f: insertMyAssignedChainsIntoPushDwnReq () : placed chain %d has S_u_len=0%", MyConfig::traceTime, chainId);
			}
			else {
				error ("traceTime=%f: insertMyAssignedChainsIntoPushDwnReq () : placed chain %d was not found in ChainMaster", MyConfig::traceTime, chainId);
			}
		}
		Chain chain2insert = chain;
		chain2insert.curLvl = lvl;
		chain2insert.potCpu = requiredCpuToLocallyPlaceChain (chain2insert);
		checkNinsertChainToList (pushDwnReq, chain2insert);
	}
}

/*************************************************************************************************************************************************
Check whether there exists (at least one) additional child to which we should send a reshuffle pkt (in async mode) - and if so, send to him.
returns true iff found a relevant child, and sent him a reshAsyncPkt.
*************************************************************************************************************************************************/
bool Datacenter::sndReshAsyncPktToNxtChild ()
{

    if (isLeaf) {
        return false;
    }
	if (MyConfig::LOG_LVL >= DETAILED_LOG && !isLeaf) {
		snprintf (buf, bufSize, "\ns%d : in sndToNxtchild. nxtChildToSndReshAsync=%d", dcId, nxtChildToSndReshAsync);
		printBufToLog ();
	}

	pushDwnReqFromChild.clear ();
	while (nxtChildToSndReshAsync < numChildren) {
		for (auto chainPtr=pushDwnReq.begin(); chainPtr!=pushDwnReq.end(); chainPtr++) {	// consider all the chains in pushDwnReq
		    if (MyConfig::DEBUG_LVL>0 && chainPtr->S_u.size() < lvl) {
		        error ("s%d : sndReshAsyncPktToNxtChild encountered in pushDwnReq c%d with S_u_len=%d while my lvl=%d", dcId, chainPtr->id, chainPtr->S_u.size(), lvl);
		    }
			if (chainPtr->S_u[lvl-1]==dcIdOfChild[nxtChildToSndReshAsync])   { /// this chain is associated with (the sub-tree of) this child
				if (!checkNinsertChainToList (pushDwnReqFromChild, *chainPtr)) {
					error ("Error in insertChainToList. See log file for details");
				}
			}
		}
		if (pushDwnReqFromChild.empty()) { // no push-down data to send to this child --> skip it
	    if (MyConfig::LOG_LVL >= DETAILED_LOG) {
	        sprintf (buf, "\ns%d : skipping child %d dcId %d - empty pushDwn req from this child", dcId, nxtChildToSndReshAsync, dcIdOfChild[nxtChildToSndReshAsync]);
		    printBufToLog ();
	    }
			nxtChildToSndReshAsync++;
			continue;
		}

		// now we know that pushDwnReqFromChild isn't empty
		if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
      sprintf (buf, "\ns%d snding reshAsync pkt to child %d, dcId=%d", dcId, nxtChildToSndReshAsync, dcIdOfChild[nxtChildToSndReshAsync]);
	    printBufToLog ();
			MyConfig::printToLog (". pushDwnReq=");
			MyConfig::printToLog (pushDwnReqFromChild);
		}
		ReshAsyncPkt* pkt2snd = new ReshAsyncPkt;

		if (this->reshInitiatorLvl==UNPLACED_LVL) {
			error ("t%f s%d has this->reshInitiatorLvl==-1", MyConfig::traceTime, dcId);
		}
		pkt2snd -> setReshInitiatorLvl (this->reshInitiatorLvl); 
		pkt2snd -> setDeficitCpu 			 	  (deficitCpu);
		pkt2snd -> setPushDwnVecArraySize (pushDwnReqFromChild.size());
		
		int idxInPushDwnVec = 0;
//		for (auto chainPtr=pushDwnReqFromChild.begin(); chainPtr!=pushDwnReqFromChild.end(); ) {	
//			pkt2snd->setPushDwnVec (idxInPushDwnVec++, *chainPtr);
//			chainPtr = pushDwnReqFromChild.erase (chainPtr);
//		}
	
		for (auto chain : pushDwnReqFromChild) {	
			pkt2snd->setPushDwnVec (idxInPushDwnVec++, chain);
		}
		sndViaQ (portToChild(nxtChildToSndReshAsync), pkt2snd); //send the pkt to the child
		nxtChildToSndReshAsync++;
		return true; // successfully sent pkt to the next child	
	}
	if (MyConfig::LOG_LVL >= DETAILED_LOG && !isLeaf) {
		snprintf (buf, bufSize, "\ns%d : finished sending to all children", dcId);
		printBufToLog ();
	}
	return false; // no additional relevant child to send to
}


/*************************************************************************************************************************************************
Initiate a print of the content of all the datacenters
*************************************************************************************************************************************************/
void Datacenter::PrintAllDatacenters ()
{
	cMessage* msg2snd = new cMessage ("PrintAllDatacentersMsg"); 
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
	MyConfig::lvlOfHighestReshDc = max (MyConfig::lvlOfHighestReshDc, lvl); // If my lvl is higher then the highest lvl reshuffled at this period - update. 

	clrRsrc ();
	for (int child(0); child<numChildren; child++) { // for each child...
	
		sndViaQ (portToChild(child), new cMessage ("prepareReshSyncMsg")); //send the bottomUPpkt to the child
	}
	
	if (isLeaf) {
		simController->preparePartialReshSync (dcId, leafId);
	}
}


/*************************************************************************************************************************************************
Rst the datacenterto prepare it for a new run of the trace:
- Cancel all events, msgs, and pkts.
- Rst all the datacenter's state variables.
- Clear all the output queues and scheduled event for packets' transmissions.
*************************************************************************************************************************************************/
void Datacenter::rst () 
{
	Enter_Method ("Datacenter::rst ()");
	fill(endXmtEvents. begin(), endXmtEvents. end(), nullptr);
	cpuCapacity   = MyConfig::cpuAtLvl[lvl]; 
	clrRsrc ();
	
	// clear all the output queues and scheduled event for packets' transmissions
	for (int portNum(0); portNum<numPorts; portNum++) { 
		outputQ[portNum].clear ();
	}
	fill(endXmtEvents. begin(), endXmtEvents. end(), nullptr); 

	rstReshAsync ();
	endFModeEvent    = nullptr; 
	isInFMode 		   = false;
	isInAccumDelay   = false;
	isInBuAccumDelay = false;
	reshuffled       = false;
}

/*************************************************************************************************************************************************
Reset the run of an async resh, and prepare for the next run
*************************************************************************************************************************************************/
void Datacenter::rstReshAsync ()
{
	reshInitiatorLvl 			 = UNPLACED_LVL; // reset the initiator of the currently run reshuffling
	nxtChildToSndReshAsync = 0; 					 // begin the passing over children from child 0
	deficitCpu						 = 0;
	pushDwnAck.					clear ();
	pushDwnReq.				  clear ();
	pushDwnReqFromChild.clear ();
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
	pushDwnReq.				clear ();
	pushDwnAck.				clear ();
	availCpu 				 = cpuCapacity;
}

/*************************************************************************************************************************************************
* Cancel previous EndFModeEvent (if exists), and Schedule a new EndFModeEvent for the current sim time + FModePeriod.
*************************************************************************************************************************************************/
void Datacenter::scheduleEndFModeEvent ()
{
	if (endFModeEvent==nullptr) { // first time need to schedule such an event --> generate a new event
		endFModeEvent = new cMessage ("endFModeEvent");			
	}
	else if (endFModeEvent->isScheduled()) { // event already exists - just need to recycle it
		endFModeEvent = cancelEvent(endFModeEvent);		
	}
	scheduleAt(simTime() + MyConfig::FModePeriod, endFModeEvent);
}

/*************************************************************************************************************************************************
 * Send the given packet.
 * If the output port is free, xmt the pkt immediately.
 * Else, queue the pkt until the output port is free, and then xmt it.
*************************************************************************************************************************************************/
void Datacenter::sndViaQ (int16_t portNum, cMessage* msg2snd)
{
	// $$ cout << "len of the msg2snd=" << msg2snd->	getBitLength() << endl; // $$$
  if (endXmtEvents[portNum]!=nullptr && endXmtEvents[portNum]->isScheduled()) { // if output Q is busy
    outputQ[portNum].insert (msg2snd);
  }
  else {
    xmt (portNum, msg2snd);
  }
}

/*************************************************************************************************************************************************
 * Xmt the given pkt to the given output port; schedule a self msg for the end of transmission.
*************************************************************************************************************************************************/
void Datacenter::xmt(int16_t portNum, cMessage* msg2snd)
{
	send(msg2snd, "port$o", portNum);

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
bool Datacenter::isPotPlaced (ChainId_t chainId) 
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
	isInFMode = true;
	scheduleEndFModeEvent (); // Restart the timer of being in F mode 
	ReshAsyncPkt *pkt = (ReshAsyncPkt*)(curHandledMsg);
	
	if (pkt->getReshInitiatorLvl ()==UNPLACED_LVL) {
		error ("t%f s%d rcvd from prnt a pkt with reshInitiatorLvl=-1", MyConfig::traceTime, dcId);
	}
	if (withinAnotherResh(pkt->getReshInitiatorLvl ())) { // if I'm within another resh, send to parent a packet with an empty pushUpAck
	
		ReshAsyncPkt* pkt2snd = new ReshAsyncPkt;
		pkt2snd -> setReshInitiatorLvl    (pkt->getReshInitiatorLvl ());
		pkt2snd -> setDeficitCpu 		      (pkt->getDeficitCpu());
		pkt2snd -> setPushDwnVecArraySize (0);
		sndViaQ (portToPrnt, pkt2snd);
		return; 
	}

	// now we know that we're not within another reshuffle 	
	if (MyConfig::DEBUG_LVL>0 && !pushDwnReq.empty()) {
		sprintf (buf, "\ns%d : rcvd ReshAsyncPktFromPrnt while pushDwnReq wasn't empty. pushDwnReq=", dcId);
		printBufToLog ();
		MyConfig::printToLog (pushDwnReq);
		error ("s%d rcvd ReshAsyncPktFromPrnt while pushDwnReq wasn't empty", dcId);
	}
	if (!pushDwnAck.empty()) {
		error ("s%d rcvd ReshAsyncPktFromPrnt while pushDwnAck wasn't empty", dcId);
	}

	this->reshInitiatorLvl = pkt->getReshInitiatorLvl ();
	this->deficitCpu = pkt->getDeficitCpu ();
	for (int i(0); i<pkt->getPushDwnVecArraySize(); i++) {
    if (!checkNinsertChainToList (pushDwnReq, pkt->getPushDwnVec(i))) {
			error ("Error in insertChainToList. See log file for details");
		}        
	}
	insertMyAssignedChainsIntoPushDwnReq ();
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
	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		sprintf (buf, "\ns%d : in handleReshAsyncPktFromChild", dcId);
		printBufToLog ();
	}
	ReshAsyncPkt *pkt = (ReshAsyncPkt*)(curHandledMsg);
	if (pkt->getReshInitiatorLvl ()==UNPLACED_LVL) {
		error ("t%f s%d rcvd from child a pkt with reshInitiatorLvl=-1", MyConfig::traceTime, dcId);
	}
	if (withinAnotherResh(pkt->getReshInitiatorLvl ())) {
		error ("rcvd from child a reshAsync pkt with reshInitiator==%d while running another resh with reshInitiatorLvl=%d", 
		pkt->getReshInitiatorLvl (), this->reshInitiatorLvl);
	}
	this->deficitCpu = pkt->getDeficitCpu ();

	// Remove from notAssigned and regain the rsrcs of chains that were pushed-down from me
	for (int i(0); i<pkt->getPushDwnVecArraySize(); i++) {
		Chain chain = pkt->getPushDwnVec(i);
		if (chain.curLvl >= lvl) { // the chain is still placed on me (or above me) - it wasn't pushed down 
			error ("s%d rcvd a reshAsync pkt from child with lvl above child's lvl", dcId);
		}
		
		// now we know that the chain is currently placed below me --> it was pushed-down from me, or from an ancestor of me.
		eraseChainFromList (pushDwnReq,  				 chain); 
		eraseChainFromVec  (notAssigned, 				 chain); // if the chain was found in notAssigned, remove it from notAssigned
		eraseChainFromList (pushDwnReqFromChild, chain);
		
		// now we know that the chain was pushed-down to a Dc below me
		if (isPotPlaced (chain.id)) {
			potPlacedChains.erase (chain.id); 
			regainRsrcOfChain		  (chain); 
		}	
		else if (isPlaced(chain.id)) { 
			placedChains.erase (chain.id); 
			regainRsrcOfChain  (chain); 
		}
		else { // now we know that the chain was pushed-down from someone else, above me --> inform my ancestors by pushing this chain to pushDwnAck
			checkNinsertChainToList (pushDwnAck, chain);
		}
	 }
	 
	if (deficitCpu <= 0) {
		if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
			sprintf (buf, "\ns%d : defCpu=%d. finishing", dcId, deficitCpu);
			printBufToLog ();
		}
		finReshAsync ();
	}
	else {
		reshAsync ();
	}
}

/*************************************************************************************************************************************************Finish the local run of a reshAsync alg':
- if I'm the initiator, come back to run bottomUp, but in "F" (feasibility) mode.
- Else, send a reshAsyncPkt to prnt.
*************************************************************************************************************************************************/
void Datacenter::finReshAsync ()
{  

	// make all the "pot-placed" chains "placed"
	for (ChainId_t chainId_t : potPlacedChains) {
		placedChains.insert (chainId_t);
	}
	if (!ChainsMaster::modifyLvl (potPlacedChains, lvl))	{
		error ("error in ChainsMaster::modifyLvl. See .log file for details.");
	}
	potPlacedChains.clear (); // in F mode, there're no "pot-placed" chains.
	if (IAmTheReshIniator()) {
		if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
			sprintf (buf, "\ns%d : finReshAsync where I'm s*", dcId);
			printBufToLog ();
		}
	}
	else {
		if (this->reshInitiatorLvl==UNPLACED_LVL) {
			error ("t%f s%d b4 calling sndReshAsyncToPrnt I have this->reshInitiatorLvl==-1", MyConfig::traceTime, dcId);
		}
		sndReshAsyncPktToPrnt ();
	}
	bottomUpFMode (); // come back to bottomUp, but in F ("feasibility") mode //$$$ maybe this should be called only if I'm the initiator?
	rstReshAsync ();
}

/*************************************************************************************************************************************************
push-down chains from the list pushDwnReq into me. 
Update state vars (availCpu, deficitCpu, placedChains) accordingly.
Return when either availCpu doesn't suffice to place any additional chain, or when deficitCpu <= 0.
*************************************************************************************************************************************************/
void Datacenter::pushDwn ()
{

	if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
		sprintf (buf, "\ns%d : chains pushed-dwn to me: ", dcId);
		printBufToLog ();
	}

	for (auto chainPtr=pushDwnReq.begin(); chainPtr!=pushDwnReq.end(); chainPtr++) {	// consider all the chains in pushDwnReq
		if (deficitCpu <= 0 || availCpu < MyConfig::minCpuToPlaceAnyChainAtLvl [lvl]) { 
			break;
		}
		if (chainPtr->curLvl < this->lvl) { 
			error ("my pushDwnReq should include only chains with curLvl >= this.lvl");
		}
		
		// If this chain is placed on me, then availCpu was already decreased, and ChainMaster was updated, when it was placed. No need for further actions
		if (isPlaced (chainPtr->id)) { 
			continue; 
		}
		
		// If this chain is potPlaced on me, then availCpu was already decreased when it was pot-placed. No need to decrease it again
		if (isPotPlaced (chainPtr->id)) { 
			// finally place this pot-placed chain
			potPlacedChains.erase   (chainPtr->id); 
			placedChains.insert 		(chainPtr->id); 

			if (!ChainsMaster::modifyLvl  (chainPtr->id, lvl)){ // inform ChainMaster about the chain's place 
				error ("Datacenter::bottomUpFMode failed to update the lvl of c%d", chainPtr->id);
			}
			if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
				snprintf (buf, bufSize, "c%d from l%d, ", chainPtr->id, chainPtr->curLvl);
				printBufToLog ();
			}
			continue; 
		}

		// now we know that this chain wasn't previously placed, or pot-placed, on me
		Cpu_t requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain (*chainPtr);

		if (availCpu >= requiredCpuToLocallyPlaceThisChain) {

			if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
				snprintf (buf, bufSize, "c%d from l%d, ", chainPtr->id, chainPtr->curLvl);
				printBufToLog ();
			}
			availCpu -= requiredCpuToLocallyPlaceThisChain;
			placedChains.insert 		(chainPtr->id); 				// locally-place the chain
			if (!ChainsMaster::modifyLvl  (chainPtr->id, lvl)){ // inform ChainMaster about the chain's place 
				error ("Datacenter::bottomUpFMode failed to update the lvl of c%d", chainPtr->id);
			}
			eraseChainFromVec (notAssigned, *chainPtr); // verify that the chain I pushed-down to myself isn't in my notAssigned list
			if (chainPtr->curLvl == reshInitiatorLvl) { // Did I push-down this chain from the initiator?
				deficitCpu -= chainPtr->potCpu;
				if (MyConfig::DEBUG_LVL>0 && requiredCpuToPlaceChainAtLvl (*chainPtr, reshInitiatorLvl) != chainPtr->potCpu) {
					error ("s%d c%d chain.potCpu=%d reshInitiatorLvl=%d, requiredCpuToPlaceChainAtLvl=%d", 
									dcId, chainPtr->id, chainPtr->potCpu, reshInitiatorLvl, requiredCpuToPlaceChainAtLvl (*chainPtr, reshInitiatorLvl));
				}
			}
			if (chainPtr->curLvl > this->lvl) { // Did I push-down this chain from an ancestor of me?
				Chain pushedDwnChain = *chainPtr; // yep --> insert info about this chain to the pushDwnAck, which I'l later send to my prnt.
				pushedDwnChain.curLvl = lvl;
				checkNinsertChainToList (pushDwnAck, pushedDwnChain);
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
		snprintf (buf, bufSize, "\ns%d : snding to prnt", dcId);
		printBufToLog ();
		if (MyConfig::LOG_LVL >= VERY_DETAILED_LOG) {
			MyConfig::printToLog (" pushDwnAck=");
			MyConfig::printToLog (pushDwnAck);
			MyConfig::printToLog (" potPlaced=");
			MyConfig::printToLog (potPlacedChains);
			MyConfig::printToLog (" PUL=");
			MyConfig::printToLog (pushUpList);
			if (isInFMode) {
				MyConfig::printToLog (" is in F");
			}
			else {
				MyConfig::printToLog (" isn't in F");
			}
		}
	}

	ReshAsyncPkt* pkt2snd = new ReshAsyncPkt;
	if (this->reshInitiatorLvl==UNPLACED_LVL) {
		error ("t%f s%d has this->reshInitiatorLvl==-1", MyConfig::traceTime, dcId);
	}

	pkt2snd -> setReshInitiatorLvl    (this->reshInitiatorLvl);
	pkt2snd -> setDeficitCpu 		      (deficitCpu);
	pkt2snd -> setPushDwnVecArraySize (pushDwnAck.size());
	int idxInPushDwnVec = 0;
	for (auto chainPtr=pushDwnAck.begin(); chainPtr!=pushDwnAck.end(); chainPtr++) {	
		pkt2snd->setPushDwnVec (idxInPushDwnVec++, *chainPtr);
	}
	sndViaQ (portToPrnt, pkt2snd);
	pushDwnAck.clear ();
}


