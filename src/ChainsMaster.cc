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
* Fill within numMigs the overall num of migrations in the last time period.
* If any chain is unplaced, return false. Else, return true.
**************************************************************************************************************************************************/
bool ChainsMaster::concludeTimePeriod (int &numMigs)
{

	numMigs = 0;
	for (auto chain : allChains) {
		if (chain.curLvl == UNPLACED_LVL) {
			return false;
		}
		if (chain.curDc != chain.S_u[chain.curLvl]) { // Was the chain migrated?
			numMigs++;
			chain.curDc = chain.S_u[chain.curLvl];
		}
	}
	return true;
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
	ChainsMaster::numInstantMigs++; // assume that every change in the lvl implies an "instantaneous migration" (several inst' mig' may happen per period).
	return true;
}

/*************************************************************************************************************************************************
* Given a chain id:
* - find the chain c having this Id. 
* - update c.S_u field according to the input pathToRoot, reflecting the up-to-date path from the user's PoA to the root.
* - If the c.curDc (current Datacenter) is still delay-feasible for this chain, leave it unchanged. Else, set c.curDc=UNPLACED_DC
* - write chain to the input by-ref parameter &modifiedChain.
* Output: true if the requested chain was found.
* 
**************************************************************************************************************************************************/
bool ChainsMaster::modifyS_u (ChainId_t chainId, const vector <DcId_t> &pathToRoot, Chain &modifiedChain)
{
	Chain dummy (chainId);
	auto chainPtr = ChainsMaster::allChains.find (dummy);

	if (MyConfig::DEBUG_LVL>0 && chainPtr==ChainsMaster::allChains.end()) {
		return false;
	}
	
	if (MyConfig::DEBUG_LVL>0) {
		return false;
	}
	modifiedChain = *chainPtr; // copy the modified chain
	modifiedChain.S_u = {pathToRoot.begin(), pathToRoot.begin()+chainPtr->mu_u_len ()}; //update the chain's S_u
	if (!(modifiedChain.dcIsDelayFeasible (modifiedChain.curDc, modifiedChain.curLvl))) {
		modifiedChain.curLvl = UNPLACED_LVL;
	}
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


