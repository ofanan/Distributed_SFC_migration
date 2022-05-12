#ifndef DATACENTER_H
#define DATACENTER_H
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>
#include "Datacenter.h"
#include "endXmtPkt_m.h"
#include "bottomUpPkt_m.h"
#include "initBottomUpMsg_m.h"
#include "pushUpPkt_m.h"
#include "prepareReshufflePkt_m.h"
#include "MyConfig.h"

using namespace omnetpp;
using namespace std;

class Datacenter : public cSimpleModule
{
  public:
  
  	// Static (not changed along a sim')
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
    int16_t id;
		static const uint16_t bufSize = 128;
		char buf[bufSize];
    
    // Dynamic
    uint16_t  availCpu;
    vector<Chain> notAssigned, pushUpVec; 
    vector<Chain> potentiallyPlacedChains;
    vector<Chain> placedChains; // For some reason, uncommenting this line makes a build-netw. error.
		uint8_t numBuMsgsRcvd; 
		
    Datacenter();
    ~Datacenter();
  	
  private:
    vector <cQueue>     outputQ;
    vector <cChannel*>  xmtChnl;
    vector <endXmtPkt*> endXmtEvents; 
    cMessage *curHandledMsg; // Incoming message that is currently handled.
    cPacket  *pkt2send; // Pkt that is currently prepared to be sent.
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
    
    void handleSelfMsg    ();
    void sendViaQ         (int16_t portNum, cPacket* pkt2send);
    void xmt              (int16_t portNum, cPacket *pkt2send);
    void handleInitBottomUpMsg ();
    void handleBottomUpPktSync ();
    void bottomUpSync     ();
    void bottomUpAsync    ();
    void pushUp           ();
    void prepareReshuffle ();
    void sendDirect       (); // send direct msgs (currently, such msgs are sent only to the traceFeeder, to inform it about chains' placement.
    void sndBottomUpPkt		();
    void sndPushUpPkt			();
};

#endif
