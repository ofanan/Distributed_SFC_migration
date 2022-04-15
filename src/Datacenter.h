#ifndef DATACENTER_H
#define DATACENTER_H
#include <stdio.h>
#include <stdint.h>

#include <vector>
#include <set>
#include <algorithm>

#include <omnetpp.h>
#include "Parameters.h"

using namespace omnetpp;
using namespace std;

class Datacenter : public cSimpleModule
{
  public:
    cModule* network; // Pointer to the network on which the simulation is running
  	string networkName;
  	int16_t lvl; // level in the tree (leaf's lvl is 0).
    int16_t numChildren;
    int16_t numParents;
    int16_t numPorts;
    int16_t idOfParent;
    vector <int16_t> idOfChildren; // idOfChildren[c] will hold the ID of child c.
    bool isRoot;
    bool isLeaf;
    int16_t  availCpu;
    set <Chain> potentiallyPlacedChains;
    set <Chain> placedChains; // For some reason, uncommenting this line makes a build-netw. error.
    Datacenter();
    ~Datacenter();
  	
  private:
    vector <cQueue>     outputQ;
    vector <cChannel*>  xmtChnl;
    vector <endXmtPkt*> endXmtEvents; // Problem: need to copy each event, and xmt it... and then remove it from the set when the event happens
    cMessage *curHandledMsg; // Incoming message that is currently handled.
    cPacket  *pkt2send; // Pkt that is currently prepared to be sent.
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
    void handleSelfMsg    ();
    void sendViaQ         (int16_t portNum);
    void xmt              (int16_t portNum);
    void bottomUp         ();
    void pushUp           ();
    void prepareReshuffle ();
    void sendDirect       (); // send direct msgs (currently, such msgs are sent only to the traceFeeder, to inform it about chains' placement.
    void handleInitBottomUpMsg ();
};

#endif
