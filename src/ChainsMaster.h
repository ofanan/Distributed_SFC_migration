#ifndef CHAINS_MASTER_H
#define CHAINS_MASTER_H

#include <omnetpp.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "MyTypes.h"
#include "Chain.h"
#include "MyConfig.h"

class Chain;
struct ChainHash;

using namespace omnetpp;
using namespace std;

class ChainsMaster {

	private:
		static const int 	bufSize = 128; // buffer for print-outs
		static char 			buf[bufSize];
		static int 				numInstantMigs; // Instantaneous num of migs, including those happen and later "cancelled" by a reshuffle in the same period.
		static unordered_map <ChainId_t, Chain> allChains; // All the currently active chains. 

	public:

	static void displaceAllChains (); 
	static int  concludeTimePeriod (int &numMigs, ChainId_t &errChainId);
	static bool eraseChains (vector <ChainId_t> vec); // a vector of chains ids to erase 
	static bool modifyS_u (ChainId_t chainId, const vector <DcId_t> &pathToRoot, Chain &modifiedChain);
	static bool modifyLvl   (ChainId_t chainId, Lvl_t newLvl); // Change t	he level of the given chain
	// inline static void eraseChain  (ChainId_t chainId); // a chain ids to erase 
	static void printAllDatacenters (int numDatacenters);
	static void printAllChains ();
	static void printAllChainsPoas (); //(bool printSu=true, bool printleaf=false, bool printCurDatacenter=false)
	static int calcNonMigCost ();
	static bool findChain (ChainId_t chainId, Chain &chain);
	static bool getChainCurDc (ChainId_t chainId, DcId_t &dcId);
	static bool insert (ChainId_t chainId, Chain chain); // insert a chain to the db
	static void clear (); // clear the db
	friend class SimController;
};

#endif

