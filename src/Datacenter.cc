#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>
#include "Parameters.h"
#include "endXmtPkt_m.h"

using namespace omnetpp;

class Datacenter : public cSimpleModule
{
public:
    int16_t numChildren;
    int16_t numParents;
    int16_t numPorts;
    bool isRoot;
    bool isLeaf;
    int16_t  availCpu;
    std::set <int32_t> assignedchains;
    std::set <int32_t> placedchains; // For some reason, uncommenting this line makes a build-netw. error.

    ~Datacenter();

private:
    std::vector <cQueue>    outputQ;
    std::vector <bool>      outputQisBusy;
    std::vector <cChannel*> xmtChnl;
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
    void xmt (cPacket *pkt, int16_t portNum);
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
    outputQisBusy.  resize (numPorts);
    xmtChnl.        resize (numPorts);
    for (int portNum (0); portNum < numPorts; portNum++) {
        xmtChnl[portNum] = gate("port$o", portNum)->getTransmissionChannel();
    }

    std::fill(outputQisBusy.begin(), outputQisBusy.end(), false);
    cPacket *pkt = new cPacket;
    xmt (pkt, 0);
}

Datacenter::~Datacenter()
{
    // cancelAndDelete all self msgs, and possibly other msgs.
//    for (int i(0); i < numParents + numChildren; i++) {
//        cancelAndDelete(endTransmissionEvent[i]);
//    }
}


void Datacenter::handleMessage (cMessage *msg)
{
    EV << "DC " << int (par ("id")) << " rcvd msg";
}

//std::string Datacenter::qNumToOutputName (int8_t qNum)
//{
//    std::string res;
//    if(isRoot) {
////        sprintf (res, "toChild[%d]$o", qNum);
//        res = "rgrgrg";
//        return res;
//    }
//
////    // Now we know that I'm not the root
////    if (qNum==0) {
////        return ("toParent[0]$o");
////    }
////    sprintf (res, "toChild[%d]$o", qNum-1);
//    return res;
//
//}


/*
 * Xmt the given packet to the given output port; schedule a self msg for the end of transmission.
 */
void Datacenter::xmt(cPacket *pkt, int16_t portNum)
{
    EV << "Starting transmission of " << pkt << endl;
    outputQisBusy[portNum] = true;

    send(pkt, "port$o", portNum);

    // Schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmissionTime = xmtChnl[portNum]->getTransmissionFinishTime();

    endXmtPkt *msg = new endXmtPkt ("");
    msg->setPortNum (portNum);

    scheduleAt(endTransmissionTime, msg);
}

