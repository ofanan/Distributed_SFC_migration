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
    std::vector <cModule*> datacenters; // pointes to all the datacenters
    std::vector <cModule*> leaves;      // pointes to all the leaves
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
  public:
    TraceFeeder ();
    ~TraceFeeder ();
};

Define_Module(TraceFeeder);

TraceFeeder::TraceFeeder()
{
}

TraceFeeder::~TraceFeeder()
{
}

void TraceFeeder::initialize ()
{
  network         = (cModule*) (getParentModule ()); // No "new", because then need to dispose it.
  numDatacenters  = (int) (network -> par ("numDatacenters"));
  numLeaves       = (int) (network -> par ("numLeaves"));
//
//    // Init the vector "leaves" with ptrs to all the leaves in the netw'
  leaves.resize (numLeaves);
  datacenters.resize (numDatacenters);
  int leaf_id = 0;
  for (int dc(0); dc<numDatacenters; dc++) {
    datacenters[dc] = network->getSubmodule("datacenters", dc);
    if (bool(datacenters[dc]->par("isLeaf"))==0) {
      leaves[leaf_id++] = datacenters[dc];
    }
  }

  RT_Chain chain0 (0);
  sendDirect (new cMessage("dummy"), leaves[0], "directMsgsPort$i");
//    EV << "sent direct msg to root\n";
//    RT_Chain rt_chain (0);


    // Discover the input gates of all the datacenters.
//    directMsgsPort
//    for (int dc(0); dc < numDatacenters; dc++) {
//      gateOfDatacenters[dc] = 
//        cGate *outGate    = gate("port$o", portNum);
//    }
//sendDirect(cMessage *msg, cModule *mod, int gateId)
//sendDirect(cMessage *msg, cModule *mod, const char *gateName, int index=-1)
//sendDirect(cMessage *msg, cGate *gate)
//cModule *targetModule = getParentModule()->getSubmodule("node2");
//sendDirect(new cMessage("msg"), targetModule, "in");
//directMsgsPort
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
