#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>
#include "Parameters.h"

using namespace omnetpp;

class Datacenter : public cSimpleModule
{
public:
    int16_t numChildren;
    int16_t numParents;
    bool isRoot;
    bool isLeaf;
    int16_t  availCpu;
    std::set <int32_t> assignedchains;
    std::set <int32_t> placedchains; // For some reason, uncommenting this line makes a build-netw. error.

    ~Datacenter();

private:
    std::vector <cQueue>    outputQ;
    std::vector <bool>      outputQisBusy;
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
    void startXmt (cPacket *pkt, int16_t portNum);
};

Define_Module(Datacenter);


void Datacenter::initialize()
{
    availCpu        = nonAugmentedCpuAtLvl[int(par("lvl"))]; // Consider rsrc aug here?
    numChildren     = (int16_t) (par("numChildren"));
    numParents      = (int16_t) (par("numParents"));
    isRoot          = (numParents==0);
    isLeaf          = (numChildren==0);
    outputQ.             resize (numParents + numChildren);
    outputQisBusy.       resize (numParents + numChildren);
    std::fill(outputQisBusy.begin(), outputQisBusy.end(), false);

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
void Datacenter::startXmt(cPacket *pkt, int16_t portNum)
{
    EV << "Starting transmission of " << pkt << endl;
    outputQisBusy[portNum] = true;

    char port_str[20];
    sprintf (port_str, "port[%d]$o", portNum);
    send(pkt, port_str); //"port[0]$o");

    // Schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmissionTime = gate("toChild[0]$o")->getTransmissionChannel()->getTransmissionFinishTime();

    // $$$ Schedule a self-msg for endXmtevent. The msg should include a filed, noting the portNum.
    //scheduleAt(endTransmissionTime, endTransmissionEvent);
}

//
//
//
