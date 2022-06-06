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


// Change the level of the given chain
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
* Given a chain id, find the respective chain within a given set of chains.
* The chain is written to foundChain.
* Output: true iff the requested chain was found.
**************************************************************************************************************************************************/
bool ChainsMaster::getChain (const ChainId_t chainId, Chain &chain)
{
	bool res = findChainInSet (ChainsMaster::allChains, chainId, chain);
	return res;
}


