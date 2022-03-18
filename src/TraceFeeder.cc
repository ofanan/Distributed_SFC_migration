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
    std::vector <cModule*> datacenters;
//    cModule* datacenters[]; // Array of pointers to the datacenters in the network
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
};

Define_Module(TraceFeeder);

void TraceFeeder::initialize ()
{
    network = (cModule*) (getParentModule ());
    numDatacenters = (int) (network -> par ("numDatacenters"));
    datacenters.resize (numDatacenters);
    for (int i(0); i<numDatacenters; i++) {
        datacenters[i] = network->getSubmodule("datacenters", i);
    }
    EV << "DC 0 has " << (int)(datacenters[0]->par("numParents")) << " parents and " << (int)(datacenters[0]->par("numChildren")) <<" children\n";
    EV << "DC 12 has " << (int)(datacenters[12]->par("numParents")) << " parents and " << (int)(datacenters[12]->par("numChildren")) <<" children\n";
}

void TraceFeeder::handleMessage (cMessage *msg)
{
}
