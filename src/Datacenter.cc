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

// Given the number of a child (0, 1, ..., numChildren-1), returns the port # connecting to this child.
inline Lvl_t Datacenter::portOfChild (const Lvl_t child) const {if (isRoot) return child; else return child+1;} 

inline void Datacenter::sndDirectToSimCtrlr (cMessage* msg) {sendDirect (msg, simController, "directMsgsPort");}

inline void	Datacenter::printStateAndEndSim () { sndDirectToSimCtrlr (new PrintStateAndEndSimMsg);}

inline void Datacenter::regainRsrcOfChain (const Chain chain) {availCpu += chain.mu_u_at_lvl(lvl); }


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

		fill(endXmtEvents. begin(), endXmtEvents. end(), nullptr);
		return;
	}
	
	// parameters that depend upon MyConfig can be initialized only after stage 0, in which MyConfig is initialized.
	if (MyConfig::mode==Async) {
		shouldSndAsyncReshPktToChild.resize (numChildren); 
		pushDownListOfChild.				 resize (numChildren); ; // pushDownListOfChild[c] will hold the pushDownList of child c
	}
	cpuCapacity   = MyConfig::cpuAtLvl[lvl]; 
  availCpu    	= cpuCapacity; // initially, all cpu rsrcs are available (no chain is assigned)

}

/*************************************************************************************************************************************************
 * Print to the log file data about chains on this DC. 
 * Inputs: 
 * - printChainIds: when true, print in a format similar to that used in the centralized alg'.
 * - printPotPlaced: when true and there're potPlacedChains to print - print them.
 * - printPushUpList: when true and pushUpList isn't impty - print them.
*************************************************************************************************************************************************/

void Datacenter::print (bool printPotPlaced, bool printPushUpList, bool printChainIds)
{
	if (placedChains.empty() && (!printPotPlaced || potPlacedChains.empty()) && (!printPushUpList || pushUpList.empty())) {
		return;
	}
	snprintf (buf, bufSize, "\ns%d : Rcs=%d, a=%d, used cpu=%d, num_of_placed_chains=%d", 
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
	else if (MyConfig::discardAllMsgs) {
		delete curHandledMsg;
		return;
	}
  else if (dynamic_cast<BottomUpPkt*>(curHandledMsg) != nullptr) {
  	if (MyConfig::mode==Sync) { 
  		handleBottomUpPktSync();
  	} 
		else {
			handleBottomUpPktAsync();		
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
  	handleReshAsyncPktFromPrnt ();
  	handleReshAsyncPktFromChild ();
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
			continue; // this chainId was potentially-placed. Hence, it's surely not in the placedChains and newlyPlacedChains
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

		// Finally, remove the chain from the list of newlyPlacedChains
		MyConfig::eraseKeyFromSet (newlyPlacedChains, 	chainId);
		
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
	newlyPlacedChains.clear ();
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
	
	if (MyConfig::LOG_LVL>=VERY_DETAILED_LOG) {
		if (pkt->getPushUpVecArraySize()==0) {
			snprintf (buf, bufSize, "\ns%d : rcvd PU pkt. pushUpVec rcvd is empty", dcId);
			printBufToLog ();
		}
		else {
			snprintf (buf, bufSize, "\ns%d : rcvd PU pkt. pushUpVec[0].id=%d pushUpVec[0].curLvl = %d", dcId, pkt->getPushUpVec(0).id, pkt->getPushUpVec(0).curLvl);
			printBufToLog ();
		}
	}
	
	for (int i(0); i< (pkt->getPushUpVecArraySize()); i++) {
		if (!insertChainToList (pushUpList, pkt->getPushUpVec (i))) {
			error ("Error in insertChainToList. See log file for details");
		}
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
		snprintf (buf, bufSize, "\ns%d : begins PU. pushUpList=", dcId);
		printBufToLog ();
		MyConfig::printToLog (pushUpList);
	}
	reshuffled = false;
	
	// Check for all chains that were pushed-up for me, and regain resources for them.
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
			placedChains.				 insert (chainPtr->id);
			newlyPlacedChains.insert (chainPtr->id);
		}
		potPlacedChains.erase (chainPtr->id);
		chainPtr = pushUpList.erase (chainPtr); // finished handling this chain pushUpList --> remove it from the pushUpList, and go on to the next chain
	}

	// Next, try to push-up chains of my descendants
	list <Chain> pushedUpChains;
	pushUpList.sort (SortChainsForPushUpList());
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
		else { // the chain is currently placed on a descendant, and I have enough place for this chain --> push up this chain to me
			availCpu 						-= requiredCpuToLocallyPlaceThisChain;
			Chain pushedUpChain  = *chainPtr; // construct a new chain to insert to placedChains, because it's forbidden to modify the chain in pushUpList
			pushedUpChain.curLvl = lvl;
			chainPtr 						 = pushUpList.erase (chainPtr); // remove the pushed-up chain from the list of potentially pushed-up chains; to be replaced by a modified chain
			placedChains.				 insert (pushedUpChain.id);
			newlyPlacedChains.insert (pushedUpChain.id);
			pushedUpChains.insert (pushedUpChains.begin(), pushedUpChain);
		}
	}
	
	for (auto chainPtr=pushedUpChains.begin(); chainPtr!=pushedUpChains.end(); chainPtr++) {
		insertChainToList (pushUpList, *chainPtr);
	}
	
	// Now, after finishing my local push-up handling, this is the final place of each chain for the next period.
	if (newlyPlacedChains.size()>0) { // inform sim_ctrlr about all the newly placed chains since the last update.
		updatePlacementInfo ();
	}

	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : finihsed PU.", dcId);
		printBufToLog ();
		print ();
	}

	if (isLeaf) {

		if (MyConfig::mode == Sync) {
			simController->finishedAlg (dcId, leafId);
		}
		
		if (DEBUG_LVL > 0 && MyConfig::mode==Sync) {
			if (!pushUpList.empty()) {
				error ("pushUpList isn't empty after running pushUp() on a leaf");
			}
		}
		return; // finished; this actually concluded the run of the BUPU alg' for the path from me to the root
	}

	genNsndPushUpPktsToChildren ();
	
	if (MyConfig::mode==Sync) { // in sync mode we're sure that we won't accept additional PUpkts at this cycle, and therefore can clear the PU list 
		pushUpList.clear();
	}
}

/*************************************************************************************************************************************************
Generate pushUpPkts, based on the data currently found in pushUpList, and xmt these pkts to all the children
*************************************************************************************************************************************************/
void Datacenter::genNsndPushUpPktsToChildren ()
{
	PushUpPkt* pkt;	 // the packet to be sent 
	
	for (int child(0); child<numChildren; child++) { // for each child...
		pkt = new PushUpPkt;
		pkt->setPushUpVecArraySize (pushUpList.size ()); // default size of pushUpVec, for case that all chains in pushUpList belong to this child; will later shrink pushUpVec otherwise 
		int idxInPushUpVec = 0;
		for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); ) {	// consider all the chains in pushUpVec
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
			sndViaQ (portOfChild(child), pkt); //send the pkt to the child
		}
		else {
			delete (pkt);
		}
	}
	if (DEBUG_LVL>0 && !pushUpList.empty()) {
		error ("pushUpList not empty after sending PU pkts to all children");
	}
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
					placedChains.				 insert  (chainPtr->id);
					newlyPlacedChains.insert  (chainPtr->id);
					chainPtr = notAssigned.erase (chainPtr);
				}
				else { // This chain can be placed higher --> potentially-place it, and insert it to the push-up list, indicating me as its current level
					potPlacedChains.insert (chainPtr->id);
					Chain modifiedChain = *chainPtr;
					modifiedChain.curLvl = lvl;
					chainPtr = notAssigned.erase (chainPtr); 
					modifiedChain.potCpu = requiredCpuToLocallyPlaceThisChain; // set the chain's "potCpu" field to the cpu required, if I'll host it
					if (!insertChainToList (pushUpList, modifiedChain)) {
						error ("Error in insertChainToList. See log file for details");
					}
				}
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
				if (MyConfig::LOG_LVL>=DETAILED_LOG) {
					snprintf (buf, bufSize, "\n************** s%d : initiating a reshuffle at lvl %d", dcId, lvl);
					printBufToLog();
				}
				if (MyConfig::mode==Sync) {
					return (MyConfig::useFullResh)? simController->prepareFullReshSync () : prepareReshSync ();
				}
				else {
					return reshAsync ();
				}
			}
		}
	}

	if (MyConfig::LOG_LVL>=DETAILED_LOG) {
		snprintf (buf, bufSize, "\ns%d : finished BU.", dcId);
		printBufToLog ();
		print ();
	}

  if (isRoot) { 
  	if (MyConfig::printBuRes) {
  		MyConfig::printToLog ("\nAfter BU:");
  		simController->printBuCost ();
  		simController->printAllDatacenters (true, false);
  	}
	  pushUp ();
  }
  else {
  	return (MyConfig::mode==Sync)? genNsndBottomUpPktSync () : genNsndBottomUpPktAsync ();
  }
}

/*************************************************************************************************************************************************
Update ChainMaster about (the IDs of) all the newly placed chains, as indicated in newlyPlacedChains.
Later, clear newlyPlacedChains.
*************************************************************************************************************************************************/
void Datacenter::updatePlacementInfo ()
{

	if (newlyPlacedChains.empty ()) {
		return;
	}
	
	for (auto chainId : newlyPlacedChains) {
		if (MyConfig::LOG_LVL == VERY_DETAILED_LOG) {
			snprintf (buf, bufSize, "\nupdating the ChainMaster: chain %d: curLvl=%d, curDC=%d\n", chainId, lvl, dcId);
			printBufToLog ();
		}
		if (!(ChainsMaster::modifyLvl (chainId, lvl))) { // Change the lvl of this chain written in our DB
			snprintf (buf, bufSize, "\nError: s%d : chain %d that appeared in a call to updatePlacementInfo was not found in ChainsMaster\n", dcId, chainId);
			printBufToLog ();
			ChainsMaster::printAllChains ();
			error ("chain %d that appeared in a call to updatePlacementInfo was not found in ChainsMaster", chainId);
		}
	}
	newlyPlacedChains.		clear ();
}


/*************************************************************************************************************************************************
Handle a bottomUP pkt, when running in Async mode.
*************************************************************************************************************************************************/
void Datacenter::handleBottomUpPktAsync ()
{
	rdBottomUpPkt ();
	bottomUp 			();

 //$$$ When do we reset notAssigned, and pushUpList?
}

/*************************************************************************************************************************************************
Read a BU pkt, and add the notAssigned chains, and the pushUpvec chains, to the respective local ("this") data bases.
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
		MyConfig::printToLog (pushUpList);
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
	int idixInPushUpVec = 0;
	for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); chainPtr++) {
		if (cannotPlaceThisChainHigher (*chainPtr)) { // if this chain cannot be placed higher, there's no use to include it in the pushUpVec to be xmtd to prnt
			continue;
		}
		
		// now we know that this chain can be placed higher --> insert it into the pushUpVec to be xmtd to prnt
		pkt2snd->setPushUpVec (idixInPushUpVec++, *chainPtr);
	}
	pkt2snd -> setPushUpVecArraySize (idixInPushUpVec); // adjust the array's size to the real number of chains inserted into it. 

	sndViaQ (0, pkt2snd); //send the bottomUPpkt to my prnt	
	if (!reshuffled) { 
		notAssigned.clear ();
	}
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
	int idixInPushUpVec = 0;
	for (auto chainPtr=pushUpList.begin(); chainPtr!=pushUpList.end(); chainPtr++) {
		if (cannotPlaceThisChainHigher (*chainPtr)) { // if this chain cannot be placed higher, there's no use to include it in the pushUpVec to be xmtd to prnt
			continue;
		}
		
		// now we know that this chain can be placed higher --> insert it into the pushUpVec to be xmtd to prnt
		pkt2snd->setPushUpVec (idixInPushUpVec++, *chainPtr);
	}
	
	if (idixInPushUpVec>0 || notAssigned.empty()) { // there's either pushUp nor notAssigned data to send to prnt
		pkt2snd -> setPushUpVecArraySize (idixInPushUpVec); // adjust the array's size to the real number of chains inserted into it. 

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
	if (idixInPushUpVec==0) { // I didn't request prnt to push-up any chain. Hence, no need to wait for his reply --> begin pushUp.
		pushUp ();
	}
}

void Datacenter::reshAsync ()
{
	error ("Sorry, async resh is not supported yet");
}

//// initiate a print of the content of all the datacenters
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
		sndViaQ (portOfChild(child), pkt); //send the bottomUPpkt to the child
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
	newlyPlacedChains.clear ();
	availCpu 				 = cpuCapacity;
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
  EV << "Starting transmission of " << pkt2snd << endl;

	send(pkt2snd, "port$o", portNum);

  // Schedule an event for the time when last bit will leave the gate.
  endXmtEvents[portNum] = new EndXmtMsg ("");
  endXmtEvents[portNum]->setPortNum (portNum);
  scheduleAt(xmtChnl[portNum]->getTransmissionFinishTime(), endXmtEvents[portNum]);
}

// return true iff the queried chain id is locally placed
bool Datacenter::checkIfChainIsPlaced (ChainId_t chainId) 
{
	auto search = placedChains.find (chainId);
	return (search!=placedChains.end()); 	
}

/*************************************************************************************************************************************************
*************************************************************************************************************************************************/
void Datacenter::handleReshAsyncPktFromPrnt  ()
{
}
/*************************************************************************************************************************************************
*************************************************************************************************************************************************/

void Datacenter::handleReshAsyncPktFromChild ()
{
}
/*************************************************************************************************************************************************
*************************************************************************************************************************************************/

void Datacenter::sndReshPktToNextChild  ()
{
}

/*************************************************************************************************************************************************
*************************************************************************************************************************************************/
void Datacenter::pushDown ()
{
}


