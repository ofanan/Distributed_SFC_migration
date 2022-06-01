#ifndef CHAINS_MASTER_H
#define CHAINS_MASTER_H

#include <stdio.h>
#include <stdint.h>

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
	
	bool  isDelayFeasibleForThisChain (const ChainId_t chainId, const DcId_t dcId, const Lvl_t lvl) const;
 	bool  cannotPlaceThisChainHigher 	(const ChainId_t chainId, const Lvl_t lvl) const; //{return chain.mu_u_len() <= this->lvl+1;}
	Cpu_t requiredCpuToPlaceChain  	 	(const ChainId_t chainId, const Lvl_t lvl) const; //{return chain.mu_u_at_lvl(lvl);}
	DcId_t getCurDatacenter 					(const ChainId_t chainId) const; // get id of the datacenter currently hosting this; or UNPLACED_DC, if not placed
  Cost_t getCpuCost 								(const ChainId_t chainId) const;
  Cpu_t  getCpu     								(const ChainId_t chainId) const; 
	
	friend class SimController;
	friend class MyConfig;
	friend class Datacenter;
};

#endif
