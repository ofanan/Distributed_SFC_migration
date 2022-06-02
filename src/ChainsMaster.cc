#include "ChainsMaster.h"

//		error ("the requested chainId %d is not found", chainId);
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


