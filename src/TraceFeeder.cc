#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>

#include "Chain.h"
#include "Parameters.h"

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
    network         = new cModule;
    network         = (cModule*) (getParentModule ());
    numDatacenters  = (int) (network -> par ("numDatacenters"));
    numLeaves       = (int) (network -> par ("numLeaves"));

    // Init the vector "leaves" with ptrs to all the leaves in the netw'
    leaves.resize (numLeaves);
    cModule *datacenter = new cModule;
    int leaf_id = 0;
    for (int i(0); i<numDatacenters; i++) {
        datacenter = network->getSubmodule("datacenters", i);
        if ((bool)(datacenter->par("isLeaf"))) {
            leaves[leaf_id++] = datacenter;
        }
    }

    // Open the trace input file

    // Schedule a self-event for beginning reading the trace
}

void TraceFeeder::handleMessage (cMessage *msg)
{
    // If self.msg:
    // - Read from the trace the data for a single slot.
    // - Init the placement alg'
    // - Schedule an event for the next run
}
