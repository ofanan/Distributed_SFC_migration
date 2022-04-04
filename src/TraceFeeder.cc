#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <iostream>
#include <fstream>

#include <vector>
#include <set>
#include <algorithm>

#include "Chain.h"
#include "Parameters.h"
#include "initBottomUpMsg_m.h"

using namespace omnetpp;
using namespace std;

class TraceFeeder : public cSimpleModule
{
  private:
    cModule* network; // Pointer to the network on which the simulation is running
    int numDatacenters;
    int numLeaves;
    std::set <Chain> allChains, newChains, crtiChains;
    std::set <int> curInts;
    std::vector <cModule*> datacenters; // pointes to all the datacenters
    std::vector <cModule*> leaves;      // pointes to all the leaves
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
  public:
    std::ofstream outFile;
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
  

  outFile.open ("example.txt");
  std::vector <int16_t> S_u = {7,2,3};
  RT_Chain chain0 (0, S_u);
  newChains.insert (chain0);
  
  initBottomUpMsg *msg2snd = new initBottomUpMsg ();
  msg2snd->setNotAssignedArraySize (1);
  msg2snd->setNotAssigned (0, {chain0});
  sendDirect (msg2snd, leaves[0], "directMsgsPort$i");
  outFile << "sent direct msg. Chain id=" << chain0.id << ", curDatacenter=" <<chain0.curDatacenter << endl; 
  cMessage *selfmsg = new cMessage ("");
  scheduleAt (simTime()+0.1, selfmsg);
  
  Chain foundChain;
  int32_t req_id = 0;
  findChainInSet (newChains, req_id, foundChain);
  outFile << "nxtDC=" << foundChain.nxtDatacenter << " \n";
}


void TraceFeeder::handleMessage (cMessage *msg)
{
  if (msg -> isSelfMessage()) {
    outFile << "rcvd self msg\n"; 
  Chain foundChain;
  int32_t req_id = 0;
  findChainInSet (newChains, req_id, foundChain);
  outFile << "nxtDC=" << foundChain.nxtDatacenter << " \n";
  }

    // If self.msg:
    // - Read from the trace the data for a single slot.
    // - Init the placement alg'
    // - Schedule an event for the next run
}


//  EV << "sent direct msg\n";
//  outFile << "Writing this to a file.\n";
//  outFile.close();

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

