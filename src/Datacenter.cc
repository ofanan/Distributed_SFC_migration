#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include "Parameters.h"

using namespace omnetpp;

class Datacenter : public cSimpleModule
{
public:
    int8_t numChildren;
    int8_t numParents;
    bool isRoot;
    bool isLeaf;
    int16_t  availCpu;
    std::vector <int32_t> assignedchains;
//    std::vector <int32_t> placedchains; // For some reason, uncommenting this line makes a build-netw. error.
    std::vector <cMessage*> endTransmissionEvent;

    ~Datacenter();

private:
//    std::vector <cQueue>    outputQ;
//    std::vector <bool>      outputQisBusy;
        virtual void initialize();
        virtual void handleMessage (cMessage *msg);
    void startXmt (cPacket *pkt, int outputQnum);
    std::string qNumToOutputName (int8_t qNum);
};

Define_Module(Datacenter);

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
//
//
//void Datacenter::startXmt(cPacket *pkt, int outputQnum)
//{
//    EV << "Starting transmission of " << pkt << endl;
//    outputQisBusy[outputQnum] = true;
//
//    std::string outputName = qNumToOutputName(outputQnum);
////    send(pkt, outputName); //"toChild[0]$o"); //outputName);
//
//    // Schedule an event for the time when last bit will leave the gate.
//    simtime_t endTransmissionTime = gate("toChild[0]$o")->getTransmissionChannel()->getTransmissionFinishTime();
//    scheduleAt(endTransmissionTime, endTransmissionEvent[outputQnum]);
//}

Datacenter::~Datacenter()
{
    for (int i(0); i < numParents + numChildren; i++) {
        cancelAndDelete(endTransmissionEvent[i]);
    }
}

void Datacenter::initialize()
{
//    availCpu        = nonAugmentedCpuAtLvl[int(par("lvl"))]; // Consider rsrc aug here?
    assignedchains = {};
//    placedchains   = {};
//    int8_t numChildren = int(par("numChildren"));
//    int8_t numParents;   //= int(par("numParents"));
//    bool isRoot;//          = (numParents==0);
//    bool isLeaf;//         = (numChildren==0);
//    outputQ.             resize (numParents + numChildren);
//    outputQisBusy.       resize (numParents + numChildren);
    endTransmissionEvent.resize (numParents + numChildren);
//    std::fill(outputQisBusy.begin(), outputQisBusy.end(), false);
    std::fill(endTransmissionEvent.begin(), endTransmissionEvent.end(), nullptr);
//
}
//
//}
//
void Datacenter::handleMessage (cMessage *msg)
{
    EV << "DC " << int (par ("id")) << " rcvd msg";
}
//
//
//
