#ifndef DATACENTER_H
#define DATACENTER_H
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>

#include "MyTypes.h"
#include "MyConfig.h"
#include "SimController.h"
#include "Chain.h"
#include "ChainsMaster.h"

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
  
  	// topology (fixed for a given simulation)
    cModule 				*network; // Pointer to the network on which the simulation is running
    SimController 	*simController;
  	string 					networkName;
  	uint8_t   			lvl; // level in the tree (leaf's lvl is 0).
    uint8_t 				numChildren;
    uint8_t 				numParents;
    uint8_t 				numPorts;
    DcId_t 					idOfParent;
    vector <DcId_t> idOfChildren; // idOfChildren[c] will hold the ID of child c.
    bool isRoot;
    bool isLeaf;
    int16_t dcId;
    int16_t leafId; // relevant only for leaves; counts the leaves from 0 to numLeaves-1
    
		//getter
		void setLeafId (DcId_t leafId);
		
    Datacenter();
    ~Datacenter();
  	
		// Functions called by the sim controller
    void rlzRsrc (vector<int32_t> IdsOfChainsToRlz);
    void initBottomUp (vector<Chain> &vecOfChainThatJoined);
    bool checkIfChainIsPlaced (ChainId_t chainId); // return true iff the queried chain id is locally placed

    // Log / debug funcs
    void print (); // print the Datacenter's content (placed and pot-placed chains, and pushUpList).
    
  private:
  	static const uint8_t 	portToPrnt=0;
  	bool 									reshuffled; // true iff this datacenter was reshuffled at this time slot (sync mode).
    vector <cQueue>     	outputQ; // Output packets queueu at each output port
    vector <cChannel*>  	xmtChnl;
    vector <EndXmtMsg*> 	endXmtEvents; // Indicates when the currently xmtd packet will finish
    cMessage 							*curHandledMsg; // Incoming message that is currently handled.
		list <Chain> 					pushUpList;     // Used by the BUPU alg'
		
    // Dynamic
    uint16_t  								 availCpu;
    vector<Chain> notAssigned, pushUpVec; 
    unordered_set <ChainId_t>  placedChains, potPlacedChains; 
    unordered_set <ChainId_t>  newlyPlacedChainsIds;    // IDs of the chains that I have placed 		 after the last update I had sent to SimCtrlr.
		uint8_t numBuPktsRcvd; 
		
		// A small buffer, used for printing results / log
		static const uint16_t bufSize = 128;
		char 	 buf[bufSize];

    virtual void initialize();
    virtual void handleMessage (cMessage *msg);

		// Functions related to the alg' running    
    inline uint16_t requiredCpuToLocallyPlaceChain 	(const Chain chain) const;
		inline bool 		isDelayFeasibleForThisChain 		(const Chain chain) const;
		inline uint8_t 	portOfChild 									 	(const uint8_t child) const; 
		inline void     sndDirectToSimCtrlr 						(cMessage* msg);
		inline void 		regainRsrcOfChain 							(const Chain  chain);
    void sndViaQ         														(int16_t portNum, cPacket* pkt2send);
    void xmt              													(int16_t portNum, cPacket *pkt2send);
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
    void genNsndPushUpPktsToChildren ();
    
    // Print functions
    inline void printBufToLog () const {MyConfig::printToLog (buf);}
    inline bool cannotPlaceThisChainHigher (const Chain chain) const;
		inline void	printStateAndEndSim 			 ();
		void 				PrintAllDatacenters 		(); // initiate a print of the content of all the datacenters
};

#endif

