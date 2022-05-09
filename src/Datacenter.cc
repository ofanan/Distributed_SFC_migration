#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>
#include "Datacenter.h"
#include "Parameters.h"
#include "endXmtPkt_m.h"
#include "bottomUpPkt_m.h"
#include "initBottomUpMsg_m.h"
#include "pushUpPkt_m.h"
#include "prepareReshufflePkt_m.h"

using namespace omnetpp;
using namespace std;

Define_Module(Datacenter);

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
	network     = (cModule*) (getParentModule ()); // No "new", because then need to dispose it.
	networkName = (network -> par ("name")).stdstringValue();
  numChildren = (int16_t) (par("numChildren"));
  numParents  = (int16_t) (par("numParents"));
  lvl				  = (int16_t) (par("lvl"));
  id					= (int16_t) (par("id"));
  availCpu    = nonAugmentedCpuAtLvl[lvl]; // Consider rsrc aug here?
        // variables: assigned chains, placedChains

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
	    idOfChildren[portNum] = int16_t (nghbr -> par ("id"));
	  }
	  else {
	    if (portNum==0) { // port 0 is towards the parents
	      idOfParent = int16_t (nghbr -> par ("id"));
	    }
	    else { // ports 1...numChildren are towards the children
	      idOfChildren[portNum-1] = int16_t (nghbr -> par ("id"));
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
    pkt2send = (cPacket*) outputQ[portNum].pop();
    xmt (portNum);
}


void Datacenter::handleMessage (cMessage *msg)
{
  curHandledMsg = msg;
  if (curHandledMsg -> isSelfMessage()) {
    handleSelfMsg ();
    return;
  }

  // Now we know that this is not a self-msg
  else if (dynamic_cast<bottomUpPkt*>(curHandledMsg) != nullptr) {
    bottomUp ();
  }
  else if (dynamic_cast<pushUpPkt*>(curHandledMsg) != nullptr) {
    pushUp ();
  }
  else if (dynamic_cast<PrepareReshufflePkt*>(curHandledMsg) != nullptr)
  {
    prepareReshuffle ();
    delete (curHandledMsg);
  }
  else if (dynamic_cast<initBottomUpMsg*>(curHandledMsg) != nullptr) {
		if (!isLeaf) {
			error ("a non-leaf datacenter received an initBottomUpMsg");
		}  
    handleInitBottomUpMsg ();
  }
  else
  {
    EV <<"BU rcvd a pkt  of an unknown type\n";
    delete (curHandledMsg);
  }
}

void Datacenter::handleInitBottomUpMsg () {

  initBottomUpMsg *msg = (initBottomUpMsg*) this->curHandledMsg;
	Chain chain;
	for (int i(0); i< (msg->getNotAssignedArraySize()); i++) {
	  chain = msg->getNotAssigned (i);
	  if (chain.mu_u[lvl] <= availCpu) {
	  	
	  }
	} 

  delete curHandledMsg;
}

void Datacenter::bottomUp ()
{
  bottomUpPkt *pkt = (bottomUpPkt*)curHandledMsg;
  EV <<"rcvd bottomUpPkt\n";
  delete (pkt);
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
void Datacenter::sendViaQ (int16_t portNum)
{
  if (endXmtEvents[portNum]!=nullptr && endXmtEvents[portNum]->isScheduled()) { // if output Q is busy
    outputQ[portNum].insert (pkt2send);
  }
  else {
    xmt (portNum);
  }
}
    /*
 * Xmt self.pkt2send to the given output port; schedule a self msg for the end of transmission.
 */
void Datacenter::xmt(int16_t portNum)
{
  EV << "Starting transmission of " << pkt2send << endl;

  send(pkt2send, "port$o", portNum);

  // Schedule an event for the time when last bit will leave the gate.
  endXmtEvents[portNum] = new endXmtPkt ("");
  endXmtEvents[portNum]->setPortNum (portNum);
  scheduleAt(xmtChnl[portNum]->getTransmissionFinishTime(), endXmtEvents[portNum]);
}

