
#include "Datacenter.h"

using namespace omnetpp;
using namespace std;

Define_Module(Datacenter);

inline bool 		Datacenter::CannotPlaceThisChainHigher 			(const Chain chain) const {return chain.mu_u_len() == this->lvl+1;}

inline uint16_t Datacenter::requiredCpuToLocallyPlaceChain 	(const Chain chain) const {return chain.mu_u_at_lvl(lvl);}

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

inline bool sortChainsByCpuUsage (const Chain &lhs, const Chain &rhs) {
	return (lhs.getCpu() < rhs.getCpu());
}

void Datacenter::initialize()
{
	network     	= (cModule*) (getParentModule ()); 
	simController = (cModule*) network->getSubmodule("sim_controller");
	networkName = (network -> par ("name")).stdstringValue();
  numChildren = (uint8_t)  (par("numChildren"));
  numParents  = (uint8_t)  (par("numParents"));
  lvl				  = (uint8_t)  (par("lvl"));
  id					= (uint16_t) (par("id"));
  availCpu    = nonAugmentedCpuAtLvl[lvl]; // Consider rsrc aug here?

  numPorts    = numParents + numChildren;
  isRoot      = (numParents==0);
  isLeaf      = (numChildren==0);
  outputQ.        resize (numPorts);
  xmtChnl.        resize (numPorts); // the xmt chnl towards each neighbor
  endXmtEvents.   resize (numPorts);
  idOfChildren.   resize (numChildren);
  numBuMsgsRcvd = 0;
  
  // Discover the xmt channels to the neighbors, and the neighbors' id's.
	for (int portNum (0); portNum < numPorts; portNum++) {
	  cGate *outGate    = gate("port$o", portNum);
	  xmtChnl[portNum]  = outGate->getTransmissionChannel();
	  cModule *nghbr    = outGate->getNextGate()->getOwnerModule();
	  if (isRoot) {
	    idOfChildren[portNum] = int16_t (nghbr -> par ("id"));
	  }
	  else {
	    if (portNum==0) { // port 0 is towards the parents
	      idOfParent = uint16_t (nghbr -> par ("id"));
	    }
	    else { // ports 1...numChildren are towards the children
	      idOfChildren[portNum-1] = uint16_t (nghbr -> par ("id"));
	    }
  	}       
  }

  fill(endXmtEvents. begin(), endXmtEvents. end(), nullptr);

}

// Print the chains placed / pot-placed in this DC.
void Datacenter::print ()
{
	snprintf (buf, bufSize, "DC %d, lvl=%d. placed chains: ", id, lvl);
	MyConfig::printToLog (buf);

	for (const Chain chain : placedChains) {
			MyConfig::printToLog (chain.id);		
	}
	
	MyConfig::printToLog ("pot. placed chains: ");
	MyConfig::printToLog (potPlacedChainsIds);
	
	MyConfig::printToLog ("\n");
}
/*
 * Currently, the only self-message is the one indicating the end of the transmission of a pkt.
 * In that case, if the relevant output queue isn't empty, the function transmits the pkt in the head of the queue.
 */
void Datacenter::handleSelfMsg ()
{
    endXmtPkt *end_xmt_pkt = (endXmtPkt*) curHandledMsg;
    int16_t portNum = end_xmt_pkt -> getPortNum();
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
  if (curHandledMsg -> isSelfMessage()) {
    handleSelfMsg ();
  }

  // Now we know that this is not a self-msg
  else if (dynamic_cast<initBottomUpMsg*>(curHandledMsg) != nullptr) {
		if (!isLeaf) {
			error ("a non-leaf datacenter received an initBottomUpMsg");
		}  
		if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
			snprintf (buf, bufSize, "DC \%d rcvd a initBU msg\n", id);
			MyConfig::printToLog (buf);
		  handleInitBottomUpMsg ();
		}
  }
  else if (dynamic_cast<bottomUpPkt*>(curHandledMsg) != nullptr) {
		if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
			snprintf (buf, bufSize, "DC \%d rcvd a BU pkt. num BU pkt rcvd=%d, numChildren=%d\n", id, numBuMsgsRcvd, numChildren);
			MyConfig::printToLog (buf);
		}
  	if (MyConfig::mode==SYNC) { handleBottomUpPktSync();} else {bottomUpAsync ();}
  }
  else if (dynamic_cast<pushUpPkt*>(curHandledMsg) != nullptr) {
  	handlePushUpPkt ();
  }
  else if (dynamic_cast<PrepareReshufflePkt*>(curHandledMsg) != nullptr)
  {
    prepareReshuffleSync ();
  }
  else
  {
    EV <<"BU rcvd a pkt  of an unknown type\n";
  }
  delete (curHandledMsg);
}

/*
Handle a rcvd initBottomUpMsg:
- Insert all the chains in the msg into this->notAssigned.
- Empty this->pushUpVec.
- Call bottomUp, for running the BU alg'.
*/
void Datacenter::handleInitBottomUpMsg () 
{

  initBottomUpMsg *msg = (initBottomUpMsg*) this->curHandledMsg;
	Chain chain;
	uint16_t mu_u;
	
	// insert all the not-assigned chains that are written in the msg into this->notAssigned vector; chains are inserted in a sorted way 
	for (int i(0); i< (msg->getNotAssignedArraySize()); i++) {
		insertSorted (this->notAssigned, msg->getNotAssigned (i));
	} 
  this -> pushUpVec = {};
	return (MyConfig::mode==SYNC)? bottomUpSync () : bottomUpAsync ();
}

/*
Handle a rcvd pushUpPkt:
- Read the data from the pkt to this->pushUpVec.
- Call pushUpSync() | pushUpAsync(), for running the PU alg'.
*/
void Datacenter::handlePushUpPkt () 
{

  pushUpPkt *pkt = (pushUpPkt*) this->curHandledMsg;
	Chain chain;
	uint16_t mu_u;
	
	// insert all the not-assigned chains that are written in the msg into this->notAssigned vector; chains are inserted in a sorted way 
	for (int i(0); i< (pkt->getPushUpVecArraySize()); i++) {
		insertSorted (this->pushUpVec, pkt->getPushUpVec (i));
	} 
	if (MyConfig::mode==SYNC){ 
//		sort pushUpVec ();
		pushUpSync ();
	}
	else {
		pushUpAsync ();
	}
}

/*
Running the PU alg'. 
Assume that this->pushUpVec already contains the relevant chains.
*/
void Datacenter::pushUpSync ()
{
	reshuffled = true;
	vector <uint16_t> newlyPlacedChains; // will hold the IDs of all the chains that this
	for (auto chainPtr=pushUpVec.begin(); chainPtr<pushUpVec.end(); chainPtr++) {

		if (potPlacedChainsIds.empty()) { // No more pot-placed chains to check                                                            
			break;
		}
		auto search = potPlacedChainsIds.find (chainPtr->id); // Look for this chain's id in my pot-placed chains 

		if (search==potPlacedChainsIds.end()) {
			continue; // this chain wasn't pot-placed by me, but by another DC.
		}
		
		potPlacedChainsIds.erase (search); // remove this chain from the vec of pot-placed chains: it will be either placed here, or already placed (pushed-up) by an ancestor
		pushUpVec.erase (chainPtr); // remove this chain from the vec of pushed-up chains: it will be either placed here, or already placed (pushed-up) by an ancestor
		
		if (chainPtr->curLvl==this->lvl) { // this chain wasn't pushed-up; need to place it here
			placedChains.insert (*chainPtr);
			newlyPlacedChains.push_back (chainPtr->id);
		}
		else { // the chain was pushed-up --> no need to reserve cpu for it anymore --> regain its resources.
			availCpu += requiredCpuToLocallyPlaceChain (*chainPtr); 
		}
	}
	
	sort (pushUpVec.begin(), pushUpVec.end(), & sortChainsByCpuUsage);
	uint16_t mu_u;
	for (uint8_t i(0); i < pushUpVec.size(); i++) {
		mu_u = requiredCpuToLocallyPlaceChain (pushUpVec[i]);
		if (mu_u <= availCpu) { // If I've enough place for this chain, then push-it up to me, and locally place it
			availCpu -= mu_u;
			pushUpVec[i].curLvl = lvl;
			placedChains.insert (pushUpVec[i]);
			newlyPlacedChains.push_back (pushUpVec[i].id);
		}
	}

	if (isLeaf) {
		// $$ Add checks; at this stage, pushUpVec should be empty
		return; // finished; this actually concluded the run of the alg'
	}
	
	sndPlacementInfoMsg (newlyPlacedChains); // inform the centrl ctrlr about the newly-placed chains

	pushUpPkt pushUpPktToChild[numChildren];	 //pushUpPktsToChild[c] will hold the packet to be sent to child c
	uint16_t pushUpVecArraySize[numChildren];
	for (uint8_t child (0); child<numChildren; child++) {pushUpVecArraySize[child]=0;} //reset the array
	Chain chain;
	for (uint8_t i(0); i < pushUpVec.size(); i++) {
		chain = pushUpVec[i];
			for (uint8_t child(0); child<numChildren; child++) {
		if (chain.S_u[lvl-1]==idOfChildren[child])   { /// find to which child this user belongs. Add a func' for that?
			pushUpPktToChild[child].setPushUpVecArraySize (++pushUpVecArraySize[child]);
			break; // found a child to associate this chain with
		}		
		error ("couldn't associate chain %d with any child\n", chain.id); 
		}
	}
	

}


/*
Running the PU alg'. 
Assume that this->pushUpVec already contains the relevant chains.
*/
void Datacenter::pushUpAsync ()
{
    pushUpPkt *pkt = (pushUpPkt*)curHandledMsg;
}

/*
Running the BU alg'. 
Assume that this->notAssigned and this->pushUpVec already contain the relevant chains, and are sorted.
*/
void Datacenter::bottomUpSync ()
{
	uint16_t mu_u; // amount of cpu required for locally placing the chain in question
	vector <uint16_t> newlyPlacedChains; // will hold the IDs of all the chains that this

	for (auto chainPtr=notAssigned.begin(); chainPtr<notAssigned.end(); chainPtr++) {
	  mu_u = chainPtr->mu_u_at_lvl(lvl);
		if (availCpu >= mu_u) {
			notAssigned.erase(chainPtr);
			availCpu -= mu_u;
			chainPtr -> curLvl = lvl;
			if (CannotPlaceThisChainHigher(*chainPtr)) { // Am I the highest delay-feasible DC of this chain?
				placedChains.insert (*chainPtr);
				newlyPlacedChains.push_back (chainPtr->id);
			}
			else {
				potPlacedChainsIds.insert (chainPtr->id);
				insertSorted (pushUpVec, *chainPtr);
			}
		}
		else if (CannotPlaceThisChainHigher(*chainPtr)) { // Am I the highest delay-feasible DC of this chain?
			prepareReshuffleSync ();
		}
	
	}

	sndPlacementInfoMsg (newlyPlacedChains);
	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		this -> print ();
	}

  return (isRoot)? pushUpSync () : sndBottomUpPkt ();
}

void Datacenter::sndPlacementInfoMsg (vector<uint16_t>  &newlyPlacedChains)
{

	uint16_t numOfNewlyPlacedChains = newlyPlacedChains.size ();
	if (numOfNewlyPlacedChains==0) {
		return; // no new chains were placed during the last run
	}
	placementInfoMsg* msg = new placementInfoMsg;

	msg -> setNewlyPlacedChainsArraySize (numOfNewlyPlacedChains);
	for (uint16_t i=0; i<numOfNewlyPlacedChains; i++) {
		msg->setNewlyPlacedChains (i, newlyPlacedChains[i]);
	}

	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "DC \%d sending placementInfoMsg\n", id);
		MyConfig::printToLog (buf);
	}
	sendDirect (msg, simController, "directMsgsPort");

}

/*
Handle a bottomUP pkt, when running in sync' mode.
- add the notAssigned chains, and the pushUpvec chains, to the respective local ("this") databases.
- delete the pkt.
- If already rcvd a bottomUp pkt from all the children, call the sync' mode of the bottom-up (BU) alg'.
*/

void Datacenter::handleBottomUpPktSync ()
{
	bottomUpPkt *pkt = (bottomUpPkt*)(curHandledMsg);
	
	// Add each chain stated in the pkt's notAssigned field into its (sorted) place in this->notAssigned()
	for (uint16_t i(0); i < (pkt->getNotAssignedArraySize ());i++) {
		insertSorted (notAssigned, pkt->getNotAssigned(i));
	}
	// Add each chain stated in the pkt's pushUpVec field into its (sorted) place in this->notAssigned()
	for (uint16_t i(0); i<pkt -> getPushUpVecArraySize (); i++) {
		pushUpVec.push_back (pkt->getPushUpVec(i)); // no need and no use to keep the push-up vector sorted for now; 
	}
	numBuMsgsRcvd++;
	if (numBuMsgsRcvd == numChildren) { // have I already rcvd a bottomUpMsg from each child?
		bottomUpSync ();
		numBuMsgsRcvd = 0;
	}
}

/*
Running the BU alg'. 
Assume that this->notAssigned and this->pushUpVec already contain the relevant chains, in the correct order.
*/
void Datacenter::bottomUpAsync ()
{
 //  bottomUpPkt *pkt = (bottomUpPkt*)curHandledMsg;
//  delete (pkt);
//	  mu_u = chain.mu_u_at_lvl(lvl);
//	  if (mu_u <= availCpu) {
//	  	chain.nxtDatacenter = id;
//	  }
	sndBottomUpPkt ();
}

void Datacenter::sndPushUpPkt () 
{
//	uint16_t i;
//	bottomUpPkt* pkt2send;
//	for (auto const childId : idOfChildren) {
//		//find the relevant chains for each child
//		
//		//Only if the number of relevant chains > 0...
//		pkt2send = new pushUpPkt;
//		pkt2send -> setPushUpVecArraySize (pushUpVec.size());
//		for (i=0; i<pushUpVec.size(); i++) {
//			pkt2send->setPushUpVec (i, pushUpVec[i]);
//		}
//	
//	}
//	
//	snprintf (buf, bufSize, "DC \%d sending a PU pkt to child\n", id);
//	MyConfig::printToLog (buf);
//	sendViaQ (0, pkt2send); //send the bottomUPpkt to my prnt
}

void Datacenter::sndBottomUpPkt ()
{
	bottomUpPkt* pkt2send = new bottomUpPkt;
	uint16_t i;

	pkt2send -> setNotAssignedArraySize (notAssigned.size());
	for (i=0; i<notAssigned.size(); i++) {
		pkt2send->setNotAssigned (i, notAssigned[i]);
	}

	pkt2send -> setPushUpVecArraySize (pushUpVec.size());
	for (i=0; i<pushUpVec.size(); i++) {
		pkt2send->setPushUpVec (i, pushUpVec[i]);
	}
	
	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "DC \%d sending a BU pkt to prnt\n", id);
		MyConfig::printToLog (buf);
	}
	sendViaQ (0, pkt2send); //send the bottomUPpkt to my prnt	
}

void Datacenter::prepareReshuffleSync () 
{
}

/*
 * Send the given packet.
 * If the output port is free, xmt the pkt immediately.
 * Else, queue the pkt until the output port is free, and then xmt it.
 */
void Datacenter::sendViaQ (int16_t portNum, cPacket* pkt2send)
{
  if (endXmtEvents[portNum]!=nullptr && endXmtEvents[portNum]->isScheduled()) { // if output Q is busy
    outputQ[portNum].insert (pkt2send);
  }
  else {
    xmt (portNum, pkt2send);
  }
}
    /*
 * Xmt self.pkt2send to the given output port; schedule a self msg for the end of transmission.
 */
void Datacenter::xmt(int16_t portNum, cPacket* pkt2send)
{
  EV << "Starting transmission of " << pkt2send << endl;

  send(pkt2send, "port$o", portNum);

  // Schedule an event for the time when last bit will leave the gate.
  endXmtEvents[portNum] = new endXmtPkt ("");
  endXmtEvents[portNum]->setPortNum (portNum);
  scheduleAt(xmtChnl[portNum]->getTransmissionFinishTime(), endXmtEvents[portNum]);
}



