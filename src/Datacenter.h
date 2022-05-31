#ifndef DATACENTER_H
#define DATACENTER_H
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>

#include "MyConfig.h"
#include "SimController.h"
#include "Chain.h"
#include "SetsOfChains.h"

#include "EndXmtMsg_m.h"
#include "BottomUpSelfMsg_m.h"
#include "BottomUpPkt_m.h"
#include "PushUpPkt_m.h"
#include "PrepareReshSyncPkt_m.h"
#include "PrintAllDatacentersMsg_m.h"
#include "PrintStateAndEndSimMsg_m.h"

using namespace omnetpp;
using namespace std;

class SimController;

class Datacenter : public cSimpleModule
{
  public:
  
    cModule *network; // Pointer to the network on which the simulation is running
//	    cModule *simController; // Pointer to the simController*/*/
    SimController *simController;
  	string networkName;
  	uint8_t   lvl; // level in the tree (leaf's lvl is 0).
    uint8_t 	numChildren;
    uint8_t 	numParents;
    uint8_t 	numPorts;
    uint16_t idOfParent;
    vector <uint16_t> idOfChildren; // idOfChildren[c] will hold the ID of child c.
    bool isRoot;
    bool isLeaf;
    int16_t dcId;
    int16_t leafId; // relevant only for leaves; counts the leaves from 0 to numLeaves-1
		static const uint16_t bufSize = 128;
		char buf[bufSize];
    
    // Dynamic
    uint16_t  availCpu;
    vector<Chain> notAssigned, pushUpVec; 
    UnorderedSetOfChains     placedChains, potPlacedChains; 
    unordered_set <ChainId_t> newlyPlacedChainsIds;    // IDs of the chains that I have placed 		 after the last update I had sent to SimCtrlr.
    unordered_set <ChainId_t> newlyDisplacedChainsIds; // IDs of the chains that I have displaceed after the last update I had sent to SimCtrlr.
		uint8_t numBuPktsRcvd; 
		
		//getter
		void setLeafId (uint16_t leafId);
		
    Datacenter();
    ~Datacenter();
  	
		// Communication with the sim controller
    void rlzRsrc (vector<int32_t> IdsOfChainsToRlz);
    void initBottomUp (vector<Chain>& vecOfChainThatJoined);

    // Log / debug funcs
    void print ();
		void				printToLog 								 (SetOfChainsOrderedByDecCpuUsage setOfChains);	
    
    
  private:
  	static const uint8_t portToPrnt=0;
  	bool 							reshuffled; // true iff this datacenter was reshuffled at this time slot (sync mode).
    vector <cQueue>     outputQ;
    vector <cChannel*>  xmtChnl;
    vector <EndXmtMsg*> endXmtEvents; 
    cMessage *curHandledMsg; // Incoming message that is currently handled.
		list <Chain> pushUpList;

    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
    
    void sndViaQ         (int16_t portNum, cPacket* pkt2send);
    void xmt              (int16_t portNum, cPacket *pkt2send);
    void handleEndXmtMsg   		  ();
    void handleBottomUpPktSync 	();
    void handlePushUpPkt			 	();
    void bottomUpSync     			();
    void bottomUpAsync  			  ();
    void pushUpSync        			();
    void pushUpAsync       			();
    void prepareReshSync		 		();
		void reshuffleAsync					();
    void genNsndBottomUpPkt			();
    void sndPushUpPkt						();
    void updatePlacementInfo 		();
    void clrRsrc 								(); // Dis-place all the placed and pot-placed chains, clear pushUpSet and notAssigned, reset availCpu
    void rlzRsrc 								(); // Release the resources
    void genNsndPushUpPktsToChildren ();
		inline bool 		isDelayFeasibleForThisChain 		(const Chain chain) const;
    
    // Print functions
    inline void printBufToLog () const {MyConfig::printToLog (buf);}
    inline bool cannotPlaceThisChainHigher (const Chain chain) const;
		inline void	printStateAndEndSim 			 ();

    inline uint16_t requiredCpuToLocallyPlaceChain 	(const Chain chain) const;
		inline uint8_t 	portOfChild 									 	(const uint8_t child) const; 
		inline void     sndDirectToSimCtrlr (cMessage* msg);
		inline void 		regainRsrcOfChain (const Chain  chain);

		// logging and debug
		void PrintAllDatacenters 		(); // initiate a print of the content of all the datacenters
};

#endif
