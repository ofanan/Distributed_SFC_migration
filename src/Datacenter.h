#ifndef DATACENTER_H
#define DATACENTER_H
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>

#include "MyConfig.h"
#include "EndXmtPkt_m.h"
#include "BottomUpPkt_m.h"
#include "InitBottomUpMsg_m.h"
#include "PlacementInfoMsg_m.h"
#include "PushUpPkt_m.h"
#include "PrepareReshSyncPkt_m.h"
#include "FinishedAlgMsg_m.h"
#include "LeftChainsMsg_m.h"

using namespace omnetpp;
using namespace std;

class Datacenter : public cSimpleModule
{
  public:
  
    cModule *network, *simController; // Pointer to the network on which the simulation is running, and to the simController*/
  	string networkName;
  	uint8_t   lvl; // level in the tree (leaf's lvl is 0).
    uint8_t 	numChildren;
    uint8_t 	numParents;
    uint8_t 	numPorts;
    uint16_t idOfParent;
    vector <uint16_t> idOfChildren; // idOfChildren[c] will hold the ID of child c.
    bool isRoot;
    bool isLeaf;
    int16_t id;
    int16_t leafId; // relevant only for leaves; counts the leaves from 0 to numLeaves-1
		static const uint16_t bufSize = 128;
		char buf[bufSize];
    
    // Dynamic
    uint16_t  availCpu;
    vector<Chain> notAssigned, pushUpVec; 
    UnorderedSetOfChains placedChains; 
    unordered_set <uint32_t> potPlacedChainsIds; //IDs of chains that are potentially-placed on me
		uint8_t numBuMsgsRcvd; 
		
    Datacenter();
    ~Datacenter();
  	
    // Log / debug funcs
    void print ();
    
  private:
  	static const uint8_t portToPrnt=0;
  	bool 							reshuffled; // true iff this datacenter was reshuffled at this time slot (sync mode).
    vector <cQueue>     outputQ;
    vector <cChannel*>  xmtChnl;
    vector <EndXmtPkt*> endXmtEvents; 
    cMessage *curHandledMsg; // Incoming message that is currently handled.
    cPacket  *pkt2send; // Pkt that is currently prepared to be sent.
		SetOfChainsOrderedByCpuUsage pushUpSet;

    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
    
    void handleSelfMsg    ();
    void sndViaQ         (int16_t portNum, cPacket* pkt2send);
    void xmt              (int16_t portNum, cPacket *pkt2send);
    void handleInitBottomUpMsg 	();
    void handleBottomUpPktSync 	();
    void handlePushUpPkt			 	();
    void bottomUpSync     			();
    void bottomUpAsync  			  ();
    void pushUpSync        			();
    void pushUpAsync       			();
    void prepareReshSync		 		();
		void reshuffleAsync					();
    void sndBottomUpPkt					();
    void sndPushUpPkt						();
    void sndPlacementInfoMsg 		(vector<uint16_t>  &newlyPlacedChains);
    void genNsndPushUpPktsToChildren ();
    inline void     printBufToLog () const {MyConfig::printToLog (buf);}
    inline bool 	  CannotPlaceThisChainHigher 		 (const Chain chain) const;
    inline uint16_t requiredCpuToLocallyPlaceChain (const Chain chain) const;
		inline uint8_t 	portOfChild 									 (const uint8_t child) const; 
};

#endif
