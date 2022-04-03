#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>
#include "Parameters.h"
#include "endXmtPkt_m.h"
#include "bottomUpPkt_m.h"
#include "initBottomUpMsg_m.h"
#include "pushUpPkt_m.h"
#include "prepareReshufflePkt_m.h"

using namespace omnetpp;

class Datacenter : public cSimpleModule
{
  public:
    int16_t numChildren;
    int16_t numParents;
    int16_t numPorts;
    int16_t idOfParent;
    std::vector <int16_t> idOfChildren;
    bool isRoot;
    bool isLeaf;
    int16_t  availCpu;
    std::set <int32_t> potentiallyPlacedChains;
    std::set <int32_t> placedChains; // For some reason, uncommenting this line makes a build-netw. error.
    Datacenter();
    ~Datacenter();

  private:
    std::vector <cQueue>     outputQ;
    std::vector <cChannel*>  xmtChnl;
    std::vector <endXmtPkt*> endXmtEvents; // Problem: need to copy each event, and xmt it... and then remove it from the set when the event happens
    cMessage *curHandledMsg; // Incoming message that is currently handled.
    cPacket  *pkt2send; // Pkt that is currently prepared to be sent.
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
    void handleSelfMsg    ();
    void sendViaQ         (int16_t portNum);
    void xmt              (int16_t portNum);
    void bottomUp         ();
    void pushUp           ();
    void prepareReshuffle ();
    void handleInitBottomUpMsg ();
};

Define_Module(Datacenter);

void Datacenter::initialize()
{
  availCpu        = nonAugmentedCpuAtLvl[int(par("lvl"))]; // Consider rsrc aug here?
  numChildren     = (int16_t) (par("numChildren"));
  numParents      = (int16_t) (par("numParents"));
  numPorts        = numParents + numChildren;
  isRoot          = (numParents==0);
  isLeaf          = (numChildren==0);

  outputQ.        resize (numPorts);
  xmtChnl.        resize (numPorts);
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
      if (portNum==0) {
        idOfParent = int16_t (nghbr -> par ("id"));
      }
      else {
        idOfChildren[portNum-1] = int16_t (nghbr -> par ("id"));
      }
    }       
  }

  std::fill(endXmtEvents. begin(), endXmtEvents. end(), nullptr);
  
  
//    // For debugging only: gen and xmt a BU pkt
//    bottomUpPkt *pkt = new bottomUpPkt();
//    pkt->setNotAssignedArraySize (1);
//    int32_t chain_id = 7;
//    std::vector <int16_t> S_u = {1,2};
//    RT_Chain chain (chain_id);
//    pkt->setNotAssigned (0, chain);
//    sendViaQ (pkt, 0);
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

//    // Now we know that the output Q isn't empty --> Pop and xmt the HoL pkt
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
  Chain chain0 = msg->getNotAssigned (0);
  chain0.nxtDatacenter = 7;
//  Chain chain0 = msg-> 
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

void Datacenter::prepareReshuffle ()
{
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

