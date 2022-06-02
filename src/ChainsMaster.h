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
	
/*	static bool   isDelayFeasibleForThisChain (const ChainId_t chainId, const DcId_t dcId, const Lvl_t lvl);*/
/* 	static bool   cannotPlaceThisChainHigher  (const ChainId_t chainId, const Lvl_t lvl); //{return chain.mu_u_len() <= this->lvl+1;}*/
/*	static Cpu_t  requiredCpuToPlaceChain  	  (const ChainId_t chainId, const Lvl_t lvl); //{return chain.mu_u_at_lvl(lvl);}*/
/*	static DcId_t getCurDatacenter 					  (const ChainId_t chainId); // get id of the datacenter currently hosting this; or UNPLACED_DC, if not placed*/
/*  static Cost_t getCpuCost 								  (const ChainId_t chainId);*/
/*  static Cpu_t  getCpu     								  (const ChainId_t chainId); */
/*  static Cpu_t  getCpu     								  (const ChainId_t chainId, const Lvl_t lvl); */
/*	static bool 	isRT_Chain									(const ChainId_t chainId);*/
	static bool 	getChain 											(const ChainId_t chainId, Chain &chain);
	
	friend class SimController;
	friend class MyConfig;
	friend class Datacenter;
};

#endif

