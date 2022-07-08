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
  	Lvl_t   			lvl; // level in the tree (leaf's lvl is 0).
    Lvl_t 				numChildren;
    Lvl_t 				numParents;
    Lvl_t 				numPorts;
    DcId_t 					idOfParent;
    vector <DcId_t> idOfChildren; // idOfChildren[c] will hold the ID of child c.
    bool isRoot;
    bool isLeaf;
    int16_t dcId;
    int16_t leafId; // relevant only for leaves; counts the leaves from 0 to numLeaves-1. For non-leaf dc, will be set to -1
		Cpu_t cpuCapacity;
		int numBuPktsRcvd; 
    
		//getter
		void setLeafId (DcId_t leafId);
		
    Datacenter();
    ~Datacenter();
  	
		// Functions called by the sim controller
    void rlzRsrc (vector<int32_t> IdsOfChainsToRlz);
    void clrRsrc 								(); // Dis-place all the placed and pot-placed chains, clear pushUpSet and notAssigned, reset availCpu
    void initBottomUp (vector<Chain> &vecOfChainThatJoined);
    bool checkIfChainIsPlaced (ChainId_t chainId); // return true iff the queried chain id is locally placed

    // Log / debug funcs
    void print (bool printPotPlaced=true, bool printPushUpList=true, bool printChainIds=true); // print the Datacenter's content (placed and pot-placed chains, and pushUpList).
    unordered_set <ChainId_t>  placedChains, potPlacedChains;
    
  private:
  	static const Lvl_t 	portToPrnt=0;
  	bool 									reshuffled; // true iff this datacenter was reshuffled at this time slot (sync mode).
    vector <cQueue>     	outputQ; // Output packets queueu at each output port
    vector <cChannel*>  	xmtChnl;
    vector <EndXmtMsg*> 	endXmtEvents; // Indicates when the currently xmtd packet will finish
    cMessage 							*curHandledMsg; // Incoming message that is currently handled.
		list <Chain> 					pushUpList;     // Used by the BUPU alg'
		
    // Dynamic
    Cpu_t  								 availCpu;
    vector<Chain> notAssigned; 
    unordered_set <ChainId_t>  newlyPlacedChains;    // IDs of the chains that I have placed after the last update I had sent to SimCtrlr.
		
		// A small buffer, used for printing results / log
		static const int bufSize = 128;
		char 	 buf[bufSize];

    void initialize(int stage);
    virtual int numInitStages() const {return 2;}; 
    virtual void handleMessage (cMessage *msg);

		// Functions related to the alg' running    
    
    Cpu_t requiredCpuToLocallyPlaceChain 			(const Chain chain) const;
		inline Lvl_t 	portOfChild 								(const Lvl_t child) const; 
		inline void     sndDirectToSimCtrlr 				(cMessage* msg);
		inline void 		regainRsrcOfChain 					(const Chain  chain);
    void sndViaQ         												(int16_t portNum, cPacket* pkt2send);
    void xmt              											(int16_t portNum, cPacket *pkt2send);
    void handleEndXmtMsg   		  ();
    void handleBottomUpPktSync 	();
    void handlePushUpPkt			 	();
    void handleAsyncReshPktFromPrnt  ();
    void handleAsyncReshPktFromChild ();
    void sndReshPktToNextChild  ();
    void bottomUpSync     			();
    void bottomUpAsync  			  ();
    void pushUpSync        			();
    void pushUpAsync       			();
    void prepareReshSync		 		();
		void reshAsync					();
    void genNsndBottomUpPkt			();
    void sndPushUpPkt						();
    void updatePlacementInfo 		();
    void genNsndPushUpPktsToChildren ();
    void pushDown ();
    
    // Print functions
    inline void printBufToLog () const {MyConfig::printToLog (buf);}
    inline bool cannotPlaceThisChainHigher (const Chain chain) const;
		inline void	printStateAndEndSim 			 ();
		void 				PrintAllDatacenters 		(); // initiate a print of the content of all the datacenters
};

#endif

