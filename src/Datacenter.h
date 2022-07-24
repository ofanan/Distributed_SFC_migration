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
#include "BottomUpPkt_m.h"
#include "PushUpPkt_m.h"
#include "PrepareReshSyncPkt_m.h"
#include "ReshAsyncPkt_m.h"

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
    vector <DcId_t> dcIdOfChild; // dcIdOfChild[c] will hold the dcId of child c.
    bool isRoot;
    bool isLeaf;
    int16_t dcId;
    int16_t leafId; // relevant only for leaves; counts the leaves from 0 to numLeaves-1. For non-leaf dc, will be set to -1
		Cpu_t cpuCapacity;
		int numBuPktsRcvd; 
  	bool 									reshuffled; // true iff this datacenter was reshuffled at this time slot (sync mode).
    
		//getter
		void setLeafId (DcId_t leafId);
		
    Datacenter();
    ~Datacenter();
  	
		// Functions called by the sim controller
    void rlzRsrc (vector<int32_t> IdsOfChainsToRlz);
    void rst (); // Rst all the datacenter's state variables to prepare it for a new run of the trace
    void clrRsrc 								(); // Dis-place all the placed and pot-placed chains, clear pushUpSet and notAssigned, reset availCpu
    void initBottomUp (vector<Chain> &vecOfChainThatJoined);
    bool isPlaced (ChainId_t chainId); // return true iff the queried chain id is locally placed
    bool isPotentiallyPlaced (ChainId_t chainId); // return true iff the queried chain id is locally potentially-placed

    // Log / debug funcs
    void print (bool printPotPlaced=true, bool printPushUpList=true, bool printChainIds=true, bool beginWithNewLine=true); // print the Datacenter's content (placed and pot-placed chains, and pushUpList).
    unordered_set <ChainId_t>  placedChains, potPlacedChains;
    
  private:
  	static const Lvl_t 	portToPrnt=0;
  	int prntGateId; // gateId of msgs arriving prnt
    vector <cQueue>     	outputQ; // Output packets queueu at each output port
    vector <cChannel*>  	xmtChnl;
    vector <int> 					gateIdToChild; // gateIdToChild[c] will hold the gateId of the gate towards child # c
    vector <EndXmtMsg*> 	endXmtEvents; // endXmts[i] will hold the event of the end of the transmission of a pkt in output channel i
		cMessage*							endFModeEvent; // will hold the event of finishing "F" mode
		bool									isInFMode; 
    cMessage 							*curHandledMsg; // Incoming message that is currently handled.
		list <Chain> 					pushUpList;     // Used by the pushUp alg'
		list <Chain> 					pushDwnReq, pushDwnAck; // List of chains requested to be pushed-down, acknowledged that were pushed-down
		Lvl_t  nxtChildToSndReshAsync; // will hold the serial num (0, 1, ..., numChildren-1) of the next child to which the Dc may try to snd a reshAsyncPkt.
		int deficitCpu;
		Lvl_t reshInitiatorLvl; // will hold the level of the initiator of the currently running async reshuffle
		
    // Dynamic
    Cpu_t  								 availCpu;
    vector<Chain> notAssigned; 
		
		// A small buffer, used for printing results / log
		static const int bufSize = 128;
		char 	 buf[bufSize];

    void initialize(int stage);
    virtual int numInitStages() const {return 2;}; 
    virtual void handleMessage (cMessage *msg);

		// Functions related to the alg' running    
    
    Cpu_t requiredCpuToLocallyPlaceChain 			  (const Chain chain) const;
    Cpu_t requiredCpuToPlaceChainAtLvl 			    (const Chain chain, Lvl_t lvl) const;
		inline Lvl_t 	portToChild 								  (const Lvl_t child) const;  // returns the index of the port towards child # c
		inline void     sndDirectToSimCtrlr 				(cMessage* msg);
		inline void 		regainRsrcOfChain 					(const Chain  chain);
		inline bool IAmTheReshIniator 							() const;
		inline bool withinResh () const;
		inline bool withinAnotherResh (const Lvl_t reshInitiatorLvl) const;
    void sndViaQ         												(int16_t portNum, cPacket* pkt2send);
    void xmt              											(int16_t portNum, cPacket *pkt2send);
    void handleEndXmtMsg   		  ();
    void handleBottomUpPktSync 	();
    void handlePushUpPkt			 	();
    void sndReshAsyncPktToPrnt  ();
    void bottomUp         			();
    void bottomUpFMode     			(); // bottom-up at "feasibility" mode
    void rdBottomUpPkt					();
    void pushUp		        			();
    void RegainRsrcOfpushedUpChains ();
    void prepareReshSync		 		();
		void initReshAsync					(); // init an async reshuffle. called upon a failure to place a chain
		void finReshAsync 					(); // finalize a run of aysnc resh on a single host
		void reshAsync							(); // run async resh. called either by initReshAsync upon a failure to place a chain, or by an arrival of reshAsyncPkt
		void rstReshAsync 					();
    void genNsndBottomUpPktSync ();
    void sndPushUpPkt						();
    void pushDwn 								();
    bool arrivedFromPrnt        (); 
    void scheduleEndFModeEvent  ();
    void handleBottomUpPktAsync ();
    void handleBottomUpPktAsyncFMode  ();
    void genNsndPushUpPktsToChildren  ();
    void handleReshAsyncPktFromPrnt   ();
    void handleReshAsyncPktFromChild  ();
    void genNsndBottomUpFmodePktAsync ();
    void genNsndBottomUpPktAsync      ();
    bool sndReshAsyncPktToNxtChild    ();
    void failedToPlaceOldChain (ChainId_t chainId);
    
    // Print functions
    inline void printBufToLog () const {MyConfig::printToLog (buf);}
    inline bool canPlaceThisChainHigher 	 (const Chain chain) const;
    inline bool cannotPlaceThisChainHigher (const Chain chain) const;
		inline void	printStateAndEndSim 			 ();
		void 				PrintAllDatacenters 		(); // initiate a print of the content of all the datacenters
};

#endif

