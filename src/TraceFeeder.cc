#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

#include "Chain.h"

using namespace omnetpp;

class TraceFeeder : public cSimpleModule
{
private:
    cModule* network; // Pointer to the network on which the simulation is running
    cModule* datacenters[]; // Array of pointers to the datacenters in the network
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
};

Define_Module(TraceFeeder);

void TraceFeeder::initialize ()
{
    network = (cModule*) (getParentModule ());
//    int submodID = network->findSubmodule("node", 0); // look up "foo[3]"
    cModule *node0 = network->getSubmodule("node", 0);
    cModule *datacenter = node0->getSubmodule("datacenter", 0);
    EV << "node 0 has " << (int)(datacenter->par("numParents")) << " parents\n";
//    node    = network -> node;
//    Chain *chain = new Chain (7);
//    EV << "The id of my chain is " << chain->id << "\n";
}

void TraceFeeder::handleMessage (cMessage *msg)
{
}
