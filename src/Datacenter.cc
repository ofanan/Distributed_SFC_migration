#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>
#include "Parameters.h"
#include "endXmtPkt_m.h"
#include "bottomUpPkt_m.h"
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
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
    void handleSelfMsg    (cMessage *msg);
    void sendViaQ         (cPacket *pkt, int16_t portNum);
    void xmt              (cPacket *pkt, int16_t portNum);
    void bottomUp         (cMessage *msg);
    void pushUp           (cMessage *msg);
    void prepareReshuffle ();
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
    for (int portNum (0); portNum < numPorts; portNum++) {
        cGate *outGate    = gate("port$o", portNum);
        xmtChnl[portNum]  = outGate->getTransmissionChannel();
        cModule *nghbr    =  outGate->getNextGate()->getOwnerModule();
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
void Datacenter::handleSelfMsg (cMessage *msg)
{
    endXmtPkt *end_xmt_pkt = (endXmtPkt*) msg;
    int16_t portNum = end_xmt_pkt -> getPortNum();
    endXmtEvents[portNum] = nullptr;
    delete (msg);
    EV << "Rcvd self msg. portNum = " << portNum;
    if (outputQ[portNum].isEmpty()) {
        return;
    }

//    // Now we know that the output Q isn't empty --> Pop and xmt the HoL pkt
    cPacket *pkt = (cPacket*) outputQ[portNum].pop();
    xmt (pkt, portNum);
}


void Datacenter::handleMessage (cMessage *msg)
{
    if (msg -> isSelfMessage()) {
        handleSelfMsg (msg);
        return;
    }

    // Now we know that this is not a self-msg
    else if (dynamic_cast<bottomUpPkt*>(msg) != nullptr)
    {
        bottomUp (msg);
    }
    else if (dynamic_cast<pushUpPkt*>(msg) != nullptr)
    {
        pushUp (msg);
    }
    else if (dynamic_cast<PrepareReshufflePkt*>(msg) != nullptr)
    {
        prepareReshuffle ();
        delete (msg);
    }
    else
    {
        EV <<"BU rcvd a pkt  of an unknown type\n";
        delete (msg);
    }

}

void Datacenter::bottomUp (cMessage *msg)
{
    bottomUpPkt *pkt = (bottomUpPkt*)msg;
    EV <<"rcvd bottomUpPkt\n";
    delete (pkt);
}

void Datacenter::pushUp (cMessage *msg)
{
    pushUpPkt *pkt = (pushUpPkt*)msg;
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
void Datacenter::sendViaQ (cPacket *pkt, int16_t portNum)
{
    if (endXmtEvents[portNum]!=nullptr && endXmtEvents[portNum]->isScheduled()) { // if output Q is busy
        outputQ[portNum].insert (pkt);
    }
    else {
        xmt (pkt, portNum);
    }
}
    /*
 * Xmt the given packet to the given output port; schedule a self msg for the end of transmission.
 */
void Datacenter::xmt(cPacket *pkt, int16_t portNum)
{
    EV << "Starting transmission of " << pkt << endl;

    send(pkt, "port$o", portNum);

    // Schedule an event for the time when last bit will leave the gate.
    endXmtEvents[portNum] = new endXmtPkt ("");
    endXmtEvents[portNum]->setPortNum (portNum);
    scheduleAt(xmtChnl[portNum]->getTransmissionFinishTime(), endXmtEvents[portNum]);
}

