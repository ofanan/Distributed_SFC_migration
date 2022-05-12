#include "Datacenter.h"

using namespace omnetpp;
using namespace std;

Define_Module(Datacenter);

inline bool Datacenter::CannotPlaceThisChainHigher (Chain chain) {return chain.mu_u_len() == this->lvl+1;}

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
	network     = (cModule*) (getParentModule ()); 
	networkName = (network -> par ("name")).stdstringValue();
  numChildren = (uint8_t)  (par("numChildren"));
  numParents  = (uint8_t)  (par("numParents"));
  lvl				  = (uint8_t)  (par("lvl"));
  id					= (uint16_t) (par("id"));
  availCpu    = nonAugmentedCpuAtLvl[lvl]; // Consider rsrc aug here?
        // variables: assigned chains, placedChains

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

/*
 * Currently, the only self-message is the one indicating the end of the transmission of a pkt.
 * In that case, if the relevant output queue isn't empty, the function transmits the pkt in the head of the queue.
 */
void Datacenter::handleSelfMsg ()
{
    endXmtPkt *end_xmt_pkt = (endXmtPkt*) curHandledMsg;
    int16_t portNum = end_xmt_pkt -> getPortNum();
    endXmtEvents[portNum] = nullptr;
    delete (curHandledMsg);
    EV << "Rcvd self msg. portNum = " << portNum;
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
    return handleSelfMsg ();
  }

  // Now we know that this is not a self-msg
  else if (dynamic_cast<initBottomUpMsg*>(curHandledMsg) != nullptr) {
		if (!isLeaf) {
			error ("a non-leaf datacenter received an initBottomUpMsg");
		}  
		snprintf (buf, bufSize, "DC \%d rcvd a initBU msg\n", id);
  	MyConfig::printToLog (buf);
    handleInitBottomUpMsg ();
  }
  else if (dynamic_cast<bottomUpPkt*>(curHandledMsg) != nullptr) {
		snprintf (buf, bufSize, "DC \%d rcvd a BU pkt. num BU pkt rcvd=%d, numChildren=%d\n", id, numBuMsgsRcvd, numChildren);
  	MyConfig::printToLog (buf);
  	return (MyConfig::mode==SYNC)? handleBottomUpPktSync () : bottomUpAsync ();
  }
  else if (dynamic_cast<pushUpPkt*>(curHandledMsg) != nullptr) {
    pushUp ();
  }
  else if (dynamic_cast<PrepareReshufflePkt*>(curHandledMsg) != nullptr)
  {
    prepareReshuffle ();
    delete (curHandledMsg);
  }
  else
  {
    EV <<"BU rcvd a pkt  of an unknown type\n";
    delete (curHandledMsg);
  }
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
  delete curHandledMsg;
  this -> pushUpVec = {};
	return (MyConfig::mode==SYNC)? bottomUpSync () : bottomUpAsync ();
}

/*
Running the BU alg'. 
Assume that this->notAssigned and this->pushUpVec already contain the relevant chains, and are sorted.
*/
void Datacenter::bottomUpSync ()
{
	uint16_t mu_u; // amount of cpu required for locally placing the chain in question
	vector <Chain> newlyPlacedChains;
	for (auto chainPtr=notAssigned.begin(); chainPtr<notAssigned.end(); chainPtr++) {
	  mu_u = chainPtr->mu_u_at_lvl(lvl);
		if (availCpu >= mu_u) {
			notAssigned.erase(chainPtr);
			availCpu -= mu_u;
			chainPtr -> curDatacenter = id;
			if (CannotPlaceThisChainHigher(*chainPtr)) { // Am I the highest delay-feasible DC of this chain?
				insertSorted (placedChains, *chainPtr);
				newlyPlacedChains.push_back (*chainPtr);
			}
			else {
				insertSorted (potPlacedChains, *chainPtr);
				insertSorted (pushUpVec, *chainPtr);
			}
		}
//		else if 
	
	}

		// $$$ Should inform traceFreeder about assignments

 	if (isRoot) {
 		sndPushUpPkt();
 	}
	else {
		sndBottomUpPkt ();
	}
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
		insertSorted (pushUpVec, pkt->getPushUpVec(i));
	}
	numBuMsgsRcvd++;
	delete (pkt);
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
	
		snprintf (buf, bufSize, "DC \%d sending a BU pkt to prnt\n", id);
  	MyConfig::printToLog (buf);
		sendViaQ (0, pkt2send); //send the bottomUPpkt to my prnt	
}

void Datacenter::pushUp ()
{
    pushUpPkt *pkt = (pushUpPkt*)curHandledMsg;
    delete (pkt);
}

void Datacenter::prepareReshuffle () {
}

// Send a direct message, e.g. to the traceFeeder, to inform about the placement of a pkt.
void Datacenter::sendDirect () {
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

