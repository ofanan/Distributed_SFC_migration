#ifndef CHAINS_MASTER_H
#define CHAINS_MASTER_H

#include <stdio.h>
#include <string.h>

#include <vector>
#include <set>
#include <unordered_set>

#include "MyTypes.h"
#include "Chain.h"

class Chain;
class ChainHash;

using namespace std;

class ChainsMaster {

		static unordered_set <Chain, ChainHash> allChains; // All the currently active chains. 

	public:

	static void eraseChains (vector <ChainId_t> vec); // a vector of chains ids to erase 
	static bool modifyLvl   (ChainId_t chainId, Lvl_t newLvl); // Change the level of the given chain
/*	inline static void eraseChain  (ChainId_t chainId); // a chain ids to erase */
	static bool 	getChain 											(const ChainId_t chainId, Chain &chain);
	
	friend class SimController;
	friend class MyConfig;
	friend class Datacenter;
};

#endif

