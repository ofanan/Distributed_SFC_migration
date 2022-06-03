#include "ChainsMaster.h"

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
	Chain chain (chainId, {});
	auto search = ChainsMaster::allChains.find (chain);

	if (search==ChainsMaster::allChains.end()) {
		return false; // chain not found - nothing to do
	}
	Chain modifiedChain 	= *search;
	modifiedChain.curLvl 	= newLvl;
	ChainsMaster::allChains.erase  (search);
	ChainsMaster::allChains.insert (modifiedChain);
	return true;
	//				numMigs++; // $$$
//			if (chain.getCurDatacenter()!=UNPLACED_DC) { // was it an old chain that migrated?
////				numMigs++; // Yep --> inc. the mig. cntr.
//		}		

}

//inline void ChainsMaster::eraseChain (ChainId_t chainId)
//{
//	eraseChainFromSet (ChainsMaster::allChains, chainId);
//}


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


