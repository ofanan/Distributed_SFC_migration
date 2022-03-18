#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>

#include "Chain.h"

using namespace omnetpp;

class TraceFeeder : public cSimpleModule
{
private:
    cModule* network; // Pointer to the network on which the simulation is running
    int numDatacenters;
    int numLeaves;
    std::vector <cModule*> leaves;
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
};

Define_Module(TraceFeeder);

void TraceFeeder::initialize ()
{
    network = new cModule;
    network = (cModule*) (getParentModule ());
    numDatacenters = (int) (network -> par ("numDatacenters"));
    numLeaves = (int) (network -> par ("numDatacenters"));
    leaves.resize (numLeaves);
    int leaf_id = 0;
    cModule *datacenter = new cModule;
    for (int i(0); i<numDatacenters; i++) {
        datacenter = network->getSubmodule("datacenters", i);
        if ((bool)(datacenter->par("isLeaf"))) {
//        if ((int)(datacenter->par("numChildren"))==0) {
            leaves[leaf_id++] = datacenter;
        }
    }
    EV << "leaf 0 has " << (int)(leaves[0]->par("numParents")) << " parents and " << (int)(leaves[0]->par("numChildren")) <<" children\n";
    EV << "leaf 3 has " << (int)(leaves[3]->par("numParents")) << " parents and " << (int)(leaves[3]->par("numChildren")) <<" children\n";
}

void TraceFeeder::handleMessage (cMessage *msg)
{
}
