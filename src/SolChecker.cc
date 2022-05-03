#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>

#include "Chain.h"

using namespace omnetpp;

class SolChecker : public cSimpleModule
{
private:
    cModule* network; // Pointer to the network on which the simulation is running
    int numDatacenters;
    std::vector <cModule*> datacenters;
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
};

Define_Module(SolChecker);

void SolChecker::initialize ()
{
    network = (cModule*) (getParentModule ());
    numDatacenters = (int) (network -> par ("numDatacenters"));
    datacenters.resize (numDatacenters);
    int leaf_id = 0;
    for (int i(0); i<numDatacenters; i++) {
        datacenters[i] = network->getSubmodule("datacenters", i);
    }
}

void SolChecker::handleMessage (cMessage *msg)
{
}
