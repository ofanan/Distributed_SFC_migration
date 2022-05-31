#include "Datacenter.h"

using namespace omnetpp;
using namespace std;

Define_Module(Datacenter);


/*************************************************************************************************************************************************
 * Infline functions
*************************************************************************************************************************************************/
inline bool sortChainsByCpuUsage (Chain lhs, Chain rhs) {return lhs.getCpu() <= rhs.getCpu();}

inline bool Datacenter::cannotPlaceThisChainHigher 	(const Chain chain) const {return chain.mu_u_len() <= this->lvl+1;}
inline bool Datacenter::isDelayFeasibleForThisChain (const Chain chain) const {return chain.mu_u_len() >= this->lvl+1;}

inline uint16_t Datacenter::requiredCpuToLocallyPlaceChain (const Chain chain) const {return chain.mu_u_at_lvl(lvl);}

// Given the number of a child (0, 1, ..., numChildren-1), returns the port # connecting to this child.
inline uint8_t Datacenter::portOfChild (const uint8_t child) const {if (isRoot) return child; else return child+1;} 

inline void Datacenter::sndDirectToSimCtrlr (cMessage* msg) {sendDirect (msg, simController, "directMsgsPort");}

inline void	Datacenter::printStateAndEndSim () { sndDirectToSimCtrlr (new PrintStateAndEndSimMsg);}

inline void Datacenter::regainRsrcOfChain (const Chain chain)   	 {availCpu += chain.mu_u_at_lvl(lvl); }

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

void Datacenter::initialize()
{
	network     	= (cModule*) (getParentModule ()); 
	simController = (SimController*) network->getSubmodule("sim_controller");
	networkName = (network -> par ("name")).stdstringValue();
  numChildren = (uint8_t)  (par("numChildren"));
  numParents  = (uint8_t)  (par("numParents"));
  lvl				  = (uint8_t)  (par("lvl"));
  dcId					= (uint16_t) (par("dcId"));
  availCpu    = nonAugmentedCpuAtLvl[lvl]; // Consider rsrc aug here?

  numPorts    = numParents + numChildren;
  isRoot      = (numParents==0);
  isLeaf      = (numChildren==0);
  outputQ.        resize (numPorts);
  xmtChnl.        resize (numPorts); // the xmt chnl towards each neighbor
  endXmtEvents.   resize (numPorts);
  idOfChildren.   resize (numChildren);
  numBuPktsRcvd = 0;
  
  // Discover the xmt channels to the neighbors, and the neighbors' id's.
	for (int portNum (0); portNum < numPorts; portNum++) {
	  cGate *outGate    = gate("port$o", portNum);
	  xmtChnl[portNum]  = outGate->getTransmissionChannel();
	  cModule *nghbr    = outGate->getNextGate()->getOwnerModule();
	  if (isRoot) {
	    idOfChildren[portNum] = int16_t (nghbr -> par ("dcId"));
	  }
	  else {
	    if (portNum==0) { // port 0 is towards the parents
	      idOfParent = uint16_t (nghbr -> par ("dcId"));
	    }
	    else { // ports 1...numChildren are towards the children
	      idOfChildren[portNum-1] = uint16_t (nghbr -> par ("dcId"));
	    }
  	}       
  }

  fill(endXmtEvents. begin(), endXmtEvents. end(), nullptr);

}

void Datacenter::printToLog (SetOfChainsOrderedByDecCpuUsage setOfChains)
{
	vector <ChainId_t> vec;
	
	for (Chain chain : setOfChains) {
		vec.insert (vec.begin(), chain.id);
	}
	
	MyConfig::printToLog (vec);
}


// Print all the chains placed and pot-placed in this DC.
void Datacenter::print ()
{
	snprintf (buf, bufSize, "\nDC %d, lvl=%d. placed chains: ", dcId, lvl);
	printBufToLog ();
	MyConfig::printToLog (placedChains);	
	MyConfig::printToLog ("pot. placed chains: ");
	MyConfig::printToLog (potPlacedChains);
	MyConfig::printToLog ("pushUpSet: ");
	printToLog (pushUpSet);
}

void Datacenter::setLeafId (uint16_t leafId)
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
  int16_t portNum 		 = endXmtMsg -> getPortNum();
  endXmtEvents[portNum] = nullptr;
  if (outputQ[portNum].isEmpty()) {
      return;
  }

  // Now we know that the output Q isn't empty --> Pop and xmt the HoL pkt
  cPacket* pkt2send = (cPacket*) outputQ[portNum].pop();
  xmt (portNum, pkt2send);
}


void Datacenter::handleMessage (cMessage *msg)
{
  curHandledMsg = msg;
  if (dynamic_cast<EndXmtMsg*>(curHandledMsg) != nullptr) {
  	handleEndXmtMsg ();
  }
  else if (dynamic_cast<BottomUpPkt*>(curHandledMsg) != nullptr) {
  	if (MyConfig::mode==SYNC) { handleBottomUpPktSync();} else {bottomUpAsync ();}
  }
  else if (dynamic_cast<PushUpPkt*>(curHandledMsg) != nullptr) {
  	handlePushUpPkt ();
  }
  else if (dynamic_cast<PrepareReshSyncPkt*>(curHandledMsg) != nullptr)
  {
    if (MyConfig::mode==SYNC) { prepareReshSync ();} {reshuffleAsync();}
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

		Chain chain;
		if (findChainInSet (potPlacedChains, chainId, chain)) {
			regainRsrcOfChain (chain);
			eraseChainFromSet (potPlacedChains,	chainId);
			continue; // if the chain was found in the potPlacedChains db, it's surely not in the placedChains and newlyPlacedChains
		}
		if (findChainInSet (placedChains, chainId, chain)) {
			regainRsrcOfChain (chain);
			eraseChainFromSet (placedChains, chainId);
		}
		MyConfig::eraseKeyFromSet (newlyPlacedChainsIds, 	chainId);
		
	}
}

/*************************************************************************************************************************************************
Initiate the bottomUpSyncAlg:
- Clear this->pushUpSet and this->notAssigned.
- Insert the chains  into this->notAssigned. The input vector is assumed to be already sorted by the delay tightness.
- Schedule a self-msg to call0 bottomUp, for running the BU alg'.
* Note: this func to be called only when the Datacenter is a leaf.
*************************************************************************************************************************************************/
void Datacenter::initBottomUp (vector<Chain>& vecOfChainThatJoined)
{

	Enter_Method ("initBottomUp (vector<Chain>& vecOfChainThatJoined)");

	if (!isLeaf) { 
		error ("Non-leaf DC %d was called by initBottomUp");
	}
	pushUpSet.clear ();	
	placedChains.clear ();
	potPlacedChains.clear ();
 	notAssigned = vecOfChainThatJoined;

 	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nDC %d received vecOfChainThatJoined=", dcId);
		printBufToLog (); 
		MyConfig::printToLog(vecOfChainThatJoined);
		print (); 
	}
  if (MyConfig::mode==SYNC) { bottomUpSync();} else {bottomUpAsync ();}		
}

/*************************************************************************************************************************************************
Handle a rcvd PushUpPkt:
- Read the data from the pkt to this->pushUpSet.
- Call pushUpSync() | pushUpAsync(), for running the PU alg'.
*************************************************************************************************************************************************/
void Datacenter::handlePushUpPkt () 
{

  PushUpPkt *pkt = (PushUpPkt*) this->curHandledMsg;
	
//	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG && dcId==5 && pkt->getPushUpVecArraySize()>0) { //$$$
//		snprintf (buf, bufSize, "\nDC %d chainId=%d, pushUpSet[0].curLvl = %d", dcId, (pkt->getPushUpVec (0)).id, (pkt->getPushUpVec (0)).curLvl);
//		printBufToLog ();
//	}


	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		if (pkt->getPushUpVecArraySize()==0) {
			snprintf (buf, bufSize, "\nDC %d rcvd PU pkt. pushUpVec rcvd is empty", dcId);
			printBufToLog ();
		}
		else {
			snprintf (buf, bufSize, "\nDC %d rcvd PU pkt. pushUpVec[0].id=%d pushUpSet[0].curLvl = %d", dcId, pkt->getPushUpVec(0).id, pkt->getPushUpVec(0).curLvl);
			printBufToLog ();
		}
	}
	for (int i(0); i< (pkt->getPushUpVecArraySize()); i++) {
		Chain chain2pushUp = pkt->getPushUpVec (i);
		if (dcId==5) {
			snprintf (buf, bufSize, "\nchain2pushUp.id=%d, chain2pushUp.curLvl=%d", chain2pushUp.id, chain2pushUp.curLvl);
			printBufToLog ();
		}
		pushUpSet.insert (chain2pushUp);
	} 

	if (MyConfig::mode==SYNC){ 
		pushUpSync ();
	}
	else {
		pushUpAsync ();
	}
}

/*************************************************************************************************************************************************
Run the PU Sync alg'. 
Assume that this->pushUpSet already contains the relevant chains.
*************************************************************************************************************************************************/
void Datacenter::pushUpSync ()
{

	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		if (pushUpSet.empty()) {
			snprintf (buf, bufSize, "\nDC %d begins PU. pushUpSet is empty", dcId);
		}
		else {
			snprintf (buf, bufSize, "\nDC %d begins PU. pushUpSet=", dcId);
		}
		printBufToLog ();
		printToLog (pushUpSet);
	}
	reshuffled = false;
	
	Chain chainInPotPlacedChains;
	
	for (auto chainInPushUpSet=pushUpSet.begin(); chainInPushUpSet!=pushUpSet.end(); ) { // for each chain in pushUpSet
		if (!findChainInSet (potPlacedChains, chainInPushUpSet->id, chainInPotPlacedChains)) { // If this chain doesn't appear in my potPlacedChains, nothing to do
			chainInPushUpSet++;
			continue;
		}	
		
		if (chainInPushUpSet->curLvl>(this->lvl) ) { // was the chain pushed-up?
			regainRsrcOfChain (*chainInPushUpSet); // Yes --> regain its resources
		}
		else { //the chain wasn't pushed-up --> need to locally place it
			chainInPotPlacedChains.curLvl = this->lvl;
			placedChains.				 insert (chainInPotPlacedChains);
			newlyPlacedChainsIds.insert (chainInPotPlacedChains.id);
		}
		eraseChainFromSet (potPlacedChains, chainInPotPlacedChains.id);
		chainInPushUpSet = pushUpSet.erase (chainInPushUpSet);
	}

	// Next, try to push-up chains of my descendants
	uint16_t requiredCpuToLocallyPlaceThisChain;
	for (auto chainPtr=pushUpSet.begin(); chainPtr!=pushUpSet.end(); ) {
		requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain (*chainPtr);
		if (chainPtr->curLvl >= lvl || // shouldn't push-up this chain either because it's already pushed-up by me/by an ancestor, ... 
				requiredCpuToLocallyPlaceThisChain > availCpu || // or because not enough avail' cpu for pushing-up, ...
				!this->isDelayFeasibleForThisChain (*chainPtr)) { // or because I'm not delay-feasible for this chain  
			chainPtr++;
			continue;
		}
		else { // the chain is currently placed on a descendant, and I have enough place for this chain --> push up this chain to me
			availCpu 						-= requiredCpuToLocallyPlaceThisChain;
			Chain pushedUpChain  = *chainPtr; // construct a new chain to insert to placedChains, because it's forbidden to modify the chain in pushUpSet
			pushedUpChain.curLvl = lvl;
			chainPtr 						 = pushUpSet.erase (chainPtr); // remove the push-upped chain from the set of potentially pushed-up chains; to be replaced by a modified chain
			placedChains.				 insert (pushedUpChain);
			newlyPlacedChainsIds.insert (pushedUpChain.id);
			pushUpSet.					 insert (pushedUpChain);
		}
	}
	
	// Now, after finishing my local push-up handling, this is the final place of each chain for the next period.
	if (newlyPlacedChainsIds.size()>0) { // inform sim_ctrlr about all the newly placed chains since the last update.
		updatePlacementInfo ();
	}

	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nDC %d finihsed PU.", dcId);
		printBufToLog ();
		print ();
	}

	if (isLeaf) {

		simController->finishedAlg (dcId, leafId);
		
		if (MyConfig::DEBUG_LVL > 0) {
			if (!pushUpSet.empty()) {
				error ("pushUpSet isn't empty after running pushUp() on a leaf");
			}
		}
		return; // finished; this actually concluded the run of the BUPU alg' for the path from me to the root
	}

	genNsndPushUpPktsToChildren ();
	pushUpSet.clear();
}

/*************************************************************************************************************************************************
Generate pushUpPkts, based on the data currently found in pushUpSet, and xmt these pkts to all the children
*************************************************************************************************************************************************/
void Datacenter::genNsndPushUpPktsToChildren ()
{
	PushUpPkt* pkt;	 // the packet to be sent 
	
	for (uint8_t child(0); child<numChildren; child++) { // for each child...
		pkt = new PushUpPkt;
		pkt->setPushUpVecArraySize (pushUpSet.size ()); // default size of pushUpVec, for case that all chains in pushUpSet belong to this child; will later shrink pushUpVec otherwise 
		uint16_t idxInPushUpVec = 0;
		for (auto chainPtr=pushUpSet.begin(); chainPtr!=pushUpSet.end(); ) {	// consider all the chains in pushUpVec
			if (chainPtr->S_u[lvl-1]==idOfChildren[child])   { /// this chain is associated with (the sub-tree of) this child
				pkt->setPushUpVec (idxInPushUpVec++, *chainPtr);
				chainPtr = pushUpSet.erase (chainPtr);
				if (MyConfig::LOG_LVL==VERY_DETAILED_LOG && dcId==2) { //$$$
					snprintf (buf, bufSize, "\nDC %d chainId=%d, pushUpVec[0].curLvl = %d", dcId, (pkt->getPushUpVec (idxInPushUpVec-1)).id, (pkt->getPushUpVec (idxInPushUpVec-1)).curLvl);
					printBufToLog ();
				}
			}
			else {
				chainPtr++;
			}
		}
		
		// shrink pushUpVec to its real size
		pkt->setPushUpVecArraySize (idxInPushUpVec);
		
		if (MyConfig::mode==SYNC || idxInPushUpVec==0) { // In sync' mode, send a pkt to each child; in async mode - send a pkt only if the child's push-up vec isn't empty
			sndViaQ (portOfChild(child), pkt); //send the bottomUPpkt to the child
		}
	}
	if (MyConfig::DEBUG_LVL>0 && !pushUpSet.empty()) {
		error ("pushUpSet not empty after sending PU pkts to all children");
	}
}

/*************************************************************************************************************************************************
Run the PU Async' alg'. 
Assume that this->pushUpSet already contains the relevant chains.
*************************************************************************************************************************************************/
void Datacenter::pushUpAsync ()
{
}

/************************************************************************************************************************************************
Running the BU alg'. 
Assume that this->notAssigned and this->pushUpSet already contain the relevant chains, and are sorted.
*************************************************************************************************************************************************/
void Datacenter::bottomUpSync ()
{

	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nDC %d beginning BU sync. notAssigned=", dcId);
		printBufToLog ();
		MyConfig::printToLog (notAssigned);
	}

	for (auto chainPtr=notAssigned.begin(); chainPtr!=notAssigned.end(); ) {
		uint16_t requiredCpuToLocallyPlaceThisChain = requiredCpuToLocallyPlaceChain(*chainPtr); 
		Chain modifiedChain; // the modified chain, to be pushed to datastructures
		if (availCpu >= requiredCpuToLocallyPlaceThisChain) { // I have enough avail' cpu for this chain --> assign it
				availCpu -= requiredCpuToLocallyPlaceThisChain;
				modifiedChain = *chainPtr;
				modifiedChain.curLvl = lvl;
				chainPtr = notAssigned.erase (chainPtr);
				if (cannotPlaceThisChainHigher(modifiedChain)) { // Am I the highest delay-feasible DC of this chain?
					placedChains.				 insert (modifiedChain);
					newlyPlacedChainsIds.insert (modifiedChain.id);
				}
				else {
					potPlacedChains.insert (modifiedChain);
					pushUpSet.			insert (modifiedChain); 
				}
		}
		else { 
			if (cannotPlaceThisChainHigher(*chainPtr)) { // Am I the highest delay-feasible DC of this chain?
				if (reshuffled) {
					snprintf (buf, bufSize, "\nDC %d: couldn't find a feasible sol' even after reshuffling", dcId);
					printBufToLog ();
					PrintAllDatacenters ();
					MyConfig::printToLog ("\n\nError: couldn't find a feasible sol' even after reshuffling");
					printStateAndEndSim  ();
				}
				return prepareReshSync ();
			}
			chainPtr++; // I don't have enough availCpu for this chain, and I'm not the highest delay-feasible DC of this chain. But maybe I've enough availCpu for the next notAssigned chain  
		}
	}

	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nDC %d finished BU sync.", dcId);
		printBufToLog ();
		print ();
	}

  if (isRoot) { 
  	pushUpSync ();
  }
  else {
  	genNsndBottomUpPkt ();
  }
}

/*************************************************************************************************************************************************
Send to the sim ctrlr a direct message, indicating (the IDs of) all the newly placed chains, as indicated in newlyPlacedChainsIds.
Later, clear newlyPlacedChainsIds.
*************************************************************************************************************************************************/
void Datacenter::updatePlacementInfo ()
{

	if (newlyPlacedChainsIds.empty () && newlyDisplacedChainsIds.empty()) {
		return;
	}
	
	Chain 	   chain;
	for (auto chainId : newlyPlacedChainsIds) {
		if (!(findChainInSet (MyConfig::allChains, chainId, chain))) {
			error ("didn't find placed chain id %d that appeared in a call to updatePlacementInfo", chainId);

		}
		else {
			if (chain.getCurDatacenter()!=UNPLACED) { // was it an old chain that migrated?
//				numMigs++; // Yep --> inc. the mig. cntr.
			}
			Chain modifiedChain (chainId, chain.S_u); // will hold the modified chain to be inserted each time
			modifiedChain.curLvl = lvl;
			
			if (MyConfig::LOG_LVL == VERY_DETAILED_LOG) {
				snprintf (buf, bufSize, "\nsimCtrlr updating: chain %d: curLvl=%d, curDC=%d\n", modifiedChain.id, modifiedChain.curLvl, modifiedChain.S_u[lvl]);
				printBufToLog ();
			}
			MyConfig::allChains.erase (chain); 					// remove the old chain from our DB
			MyConfig::allChains.insert (modifiedChain); // insert the modified chain, with the updated place (level) into our DB
		}
	}


	newlyPlacedChainsIds.		clear ();
	newlyDisplacedChainsIds.clear ();
}


/*************************************************************************************************************************************************
Handle a bottomUP pkt, when running in sync' mode.
- add the notAssigned chains, and the pushUpvec chains, to the respective local ("this") databases.
- delete the pkt.
- If already rcvd a bottomUp pkt from all the children, call the sync' mode of the bottom-up (BU) alg'.
*************************************************************************************************************************************************/
void Datacenter::handleBottomUpPktSync ()
{

	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nDC %d handling a BU pkt. src=%d", dcId, ((Datacenter*) curHandledMsg->getSenderModule())->dcId);
		printBufToLog ();
	}

	if (numBuPktsRcvd==0) { // this is the first BU pkt rcvd from a child at this period
		notAssigned.clear ();
		pushUpSet.  clear ();
	}
	numBuPktsRcvd++;
	
	BottomUpPkt *pkt = (BottomUpPkt*)(curHandledMsg);
	
	// Add each chain stated in the pkt's notAssigned field into its (sorted) place in this->notAssigned()
	for (uint16_t i(0); i < (pkt->getNotAssignedArraySize ());i++) {
		insertSorted (notAssigned, pkt->getNotAssigned(i));
	}
	
	// Add each chain stated in the pkt's pushUpVec field into this->pushUpSet
	for (uint16_t i(0); i<pkt -> getPushUpVecArraySize (); i++) {
        pushUpSet.insert (pkt->getPushUpVec(i));
		if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
			snprintf (buf, bufSize, "\nDC %d inserted chain %d to pushUpSet. PushUpSetSize=%d", dcId, pkt->getPushUpVec(i).id, (int)pushUpSet.size());
			printBufToLog ();
		}
	}
	if (MyConfig::LOG_LVL == VERY_DETAILED_LOG) {
        snprintf (buf, bufSize, "\nDC %d pushUpSet=", dcId);
		printBufToLog ();
		printToLog (pushUpSet);
	}
	if (numBuPktsRcvd == numChildren) { // have I already rcvd a bottomUpMsg from each child?
		bottomUpSync ();
		numBuPktsRcvd = 0;
	}
}

/*************************************************************************************************************************************************
Running the BU alg'. 
Assume that this->notAssigned and this->pushUpSet already contain the relevant chains, in the correct order.
*************************************************************************************************************************************************/
void Datacenter::bottomUpAsync ()
{
}

/*************************************************************************************************************************************************
Generate a BottomUpPkt, based on the data currently found in notAssigned and pushUpSet, and xmt it to my parent:
- For each chain in this->pushUpSet:
	- if the chain can be placed higher, include it in pushUpSet to be xmtd to prnt, and remove it from the this->pushUpSet.
For each chain in this->notAssigned:
	- insert the chain into the "notAssigned" field in the pkt to be xmtd to prnt, and remove it from this->notAssigned.
*************************************************************************************************************************************************/
void Datacenter::genNsndBottomUpPkt ()
{
	BottomUpPkt* pkt2send = new BottomUpPkt;

	pkt2send -> setNotAssignedArraySize (notAssigned.size());
	for (uint16_t i=0; i<notAssigned.size(); i++) {
		pkt2send->setNotAssigned (i, notAssigned[i]);
	}

	pkt2send -> setPushUpVecArraySize (pushUpSet.size()); // allocate default size of pushUpVec; will shrink it later to the exact required size.
	uint16_t idixInPushUpVec = 0;
	for (auto chainPtr=pushUpSet.begin(); chainPtr!=pushUpSet.end(); chainPtr++) {
		if (cannotPlaceThisChainHigher (*chainPtr)) { // if this chain cannot be placed higher, there's no use to include it in the pushUpVec to be xmtd to prnt
			continue;
		}
		
		// now we know that this chain can be placed higher --> insert it into the pushUpVec to be xmtd to prnt
		pkt2send->setPushUpVec (idixInPushUpVec++, *chainPtr);
	}
	pkt2send -> setPushUpVecArraySize (idixInPushUpVec); // adjest the array's size to the real number of chains inserted into it. 

	sndViaQ (0, pkt2send); //send the bottomUPpkt to my prnt	
	if (!reshuffled) { 
		notAssigned.clear ();
	}
}

void Datacenter::reshuffleAsync ()
{
}

//// initiate a print of the content of all the datacenters
void Datacenter::PrintAllDatacenters ()
{
	PrintAllDatacentersMsg* msg2snd = new PrintAllDatacentersMsg; 
	sendDirect (msg2snd, simController, "directMsgsPort");
}

void Datacenter::prepareReshSync () 
{
	if (reshuffled) {
		genNsndBottomUpPkt ();	
	}
	reshuffled = true;
	clrRsrc ();
	for (uint8_t child(0); child<numChildren; child++) { // for each child...
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
- Clear notAssigned and pushUpSet.
- reset availCpu.
*************************************************************************************************************************************************/
void Datacenter::clrRsrc () 
{
	notAssigned. 					clear ();
	pushUpSet.   					clear ();
	placedChains.			 	  clear ();
	potPlacedChains.			clear ();
	newlyPlacedChainsIds.	clear ();
	availCpu 									 = 0;
}

/*************************************************************************************************************************************************
 * Send the given packet.
 * If the output port is free, xmt the pkt immediately.
 * Else, queue the pkt until the output port is free, and then xmt it.
*************************************************************************************************************************************************/
void Datacenter::sndViaQ (int16_t portNum, cPacket* pkt2send)
{
  if (endXmtEvents[portNum]!=nullptr && endXmtEvents[portNum]->isScheduled()) { // if output Q is busy
    outputQ[portNum].insert (pkt2send);
  }
  else {
    xmt (portNum, pkt2send);
  }
}

/*************************************************************************************************************************************************
 * Xmt self.pkt2send to the given output port; schedule a self msg for the end of transmission.
*************************************************************************************************************************************************/
void Datacenter::xmt(int16_t portNum, cPacket* pkt2send)
{
  EV << "Starting transmission of " << pkt2send << endl;

	send(pkt2send, "port$o", portNum);

  // Schedule an event for the time when last bit will leave the gate.
  endXmtEvents[portNum] = new EndXmtMsg ("");
  endXmtEvents[portNum]->setPortNum (portNum);
  scheduleAt(xmtChnl[portNum]->getTransmissionFinishTime(), endXmtEvents[portNum]);
}
