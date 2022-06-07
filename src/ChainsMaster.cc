#include "ChainsMaster.h"

unordered_set <Chain, ChainHash> 	ChainsMaster::allChains;
int ChainsMaster::numInstantMigs; // Instantaneous num of migs, including those happen and later "cancelled" by a reshuffle in the same period.
int ChainsMaster::numMigs;

void ChainsMaster::eraseChains (vector <ChainId_t> vec)
{
	for (auto chainId : vec) {
		eraseChainFromSet (allChains, chainId);
	}
}


/*************************************************************************************************************************************************
* Given a chain id, update the curLvl field of the respective chain to the given newLvl.
* Output: true iff the requested chain was found.
**************************************************************************************************************************************************/
bool ChainsMaster::modifyLvl (ChainId_t chainId, Lvl_t newLvl)
{
	Chain chain (chainId);
	auto search = ChainsMaster::allChains.find (chain);

	if (search==ChainsMaster::allChains.end()) {
		return false; // chain not found - nothing to do
	}
	Chain modifiedChain 	= *search;
	modifiedChain.curLvl 	= newLvl;
	ChainsMaster::allChains.erase  (search);
	ChainsMaster::allChains.insert (modifiedChain);
	if (chain.getCurDatacenter()!=UNPLACED_DC) { // was it an old chain that migrated?
		ChainsMaster::numInstantMigs++;
	}		
	return true;
}

/*************************************************************************************************************************************************
* Given a chain id, update the curLvl field of the respective chain to the given newLvl.
* Output: true if the requested chain was found AND it is already placed.
**************************************************************************************************************************************************/
bool ChainsMaster::modifyS_u (ChainId_t chainId, const vector <DcId_t> &pathToRoot, Chain &modifiedChain, DcId_t &curDatacenter)
{
	Chain dummy (chainId);
	auto chainPtr = ChainsMaster::allChains.find (dummy);

	if (MyConfig::DEBUG_LVL>0 && chainPtr==ChainsMaster::allChains.end()) {
		return false;
	}
	
	curDatacenter = chainPtr->getCurDatacenter();
	if (MyConfig::DEBUG_LVL>0 && curDatacenter==UNPLACED_DC) {
		return false;
	}
	modifiedChain = *chainPtr; // copy the modified chain
	modifiedChain.S_u = {pathToRoot.begin(), pathToRoot.begin()+chainPtr->mu_u_len ()}; //update the chain's S_u
	allChains.erase (chainPtr); // remove the chain from our DB; will soon re-write it to the DB, having updated fields
	allChains.insert (modifiedChain);
	return true;
}

/*************************************************************************************************************************************************
* Given a chain id, find the respective chain within a given set of chains.
* The chain is written to foundChain.
* Output: true iff the requested chain was found.
**************************************************************************************************************************************************/
bool ChainsMaster::getChain (const ChainId_t chainId, Chain &chain)
{
	bool res = findChainInSet (ChainsMaster::allChains, chainId, chain);
	return res;
}


