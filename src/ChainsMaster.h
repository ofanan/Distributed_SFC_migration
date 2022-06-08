#ifndef CHAINS_MASTER_H
#define CHAINS_MASTER_H

#include <omnetpp.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <set>
#include <unordered_set>

#include "MyTypes.h"
#include "Chain.h"
#include "MyConfig.h"

class Chain;
class ChainHash;

using namespace omnetpp;
using namespace std;

class ChainsMaster {

	private:
		static unordered_set <Chain, ChainHash> allChains; // All the currently active chains. 
		static int numInstantMigs; // Instantaneous num of migs, including those happen and later "cancelled" by a reshuffle in the same period.
		static int numMigs;
/*  	inline void printBufToLog () {MyConfig::printToLog(buf);}*/
		static const uint16_t bufSize = 128;
		static char buf[bufSize];

	public:

	static bool concludeTimePeriod (int &numMigs);
	static void eraseChains (vector <ChainId_t> vec); // a vector of chains ids to erase 
	static bool modifyS_u (ChainId_t chainId, const vector <DcId_t> &pathToRoot, Chain &modifiedChain);
	static bool modifyLvl   (ChainId_t chainId, Lvl_t newLvl); // Change the level of the given chain
	// inline static void eraseChain  (ChainId_t chainId); // a chain ids to erase 
	static bool getChain 											(const ChainId_t chainId, Chain &chain);
	static void printAllDatacenters (int numDatacenters);
	static void printAllChains ();
	static void printAllChainsPoas (); //(bool printSu=true, bool printleaf=false, bool printCurDatacenter=false)
	static int calcNonMigCost ();
	friend class SimController;
/*	friend class MyConfig;*/
};

#endif

