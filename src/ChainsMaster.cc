#include "ChainsMaster.h"

unordered_set <Chain, ChainHash> ChainsMaster::allChains;
unordered_map <ChainId_t, Chain> ChainsMaster::allChains_; // All the currently active chains. 
int ChainsMaster::numInstantMigs; // Instantaneous num of migs, including those happen and later "cancelled" by a reshuffle in the same period.
int ChainsMaster::numMigs;
const int ChainsMaster::bufSize;
char ChainsMaster::buf[bufSize];

void ChainsMaster::eraseChains (vector <ChainId_t> vec)
{
	for (auto chainId : vec) {
		ChainsMaster::allChains_.erase (chainId);
	}
}

/*************************************************************************************************************************************************
* Inserts the chain, given its id, into the by-ref argument chain.
* Returns false the ChainId_t was already found in allChains.
**************************************************************************************************************************************************/
bool ChainsMaster::insert (ChainId_t chainId, Chain chain)
{
	if (MyConfig::DEBUG_LVL>1) {
		auto it = ChainsMaster::allChains_.find(chainId);
		if (it == ChainsMaster::allChains_.end()) { 
			return false;
		}
	}
	ChainsMaster::allChains_.insert ({chainId, chain}); 
	return true;
}


/*************************************************************************************************************************************************
* Finds the chain, given its id, into the by-ref argument chain.
* Returns true iff the ChainId_t is found in allChains.
**************************************************************************************************************************************************/
bool ChainsMaster::findChain (ChainId_t chainId, Chain &chain)
{
	auto it = ChainsMaster::allChains_.find(chainId);
	if (it == ChainsMaster::allChains_.end()) { 
		return false;
  }
  chain = it->second;
  return true;
}


/*************************************************************************************************************************************************
* Given a chain id, update the curLvl field of the respective chain to the given newLvl.
* Output: true iff the requested chain was found.
**************************************************************************************************************************************************/
bool ChainsMaster::modifyLvl (ChainId_t chainId, Lvl_t newLvl)
{
	auto it = ChainsMaster::allChains_.find(chainId);
	if (it == ChainsMaster::allChains_.end()) { 
		return false;
  }
  it->second.curLvl = newLvl;
	ChainsMaster::numInstantMigs++; // assume that every change in the lvl implies an "instantaneous migration" (several inst' mig' may happen per period).
  return true;
}


/*************************************************************************************************************************************************
* Given a chain id:
* - find the chain c having this Id. 
* - update c.S_u field according to the input pathToRoot, reflecting the up-to-date path from the user's PoA to the root.
* - If current datacenter hosting this chain is still delay-feasible for this it, leave it unchanged. Else, set c.curLvl=UNPLACED_LVL, to  indicate
		that this chain should be migrated.
* - write chain to the input by-ref parameter &modifiedChain.
* Output: true if the requested chain was found.
* 
**************************************************************************************************************************************************/
bool ChainsMaster::modifyS_u (ChainId_t chainId, const vector <DcId_t> &pathToRoot, Chain &modifiedChain)
{

	auto it = ChainsMaster::allChains_.find(chainId);
	if (it == ChainsMaster::allChains_.end()) { 
		return false;
  }

	it->second.S_u = {pathToRoot.begin(), pathToRoot.begin()+it->second.mu_u_len ()}; //update the chain's S_u
	if (!(it->second.dcIsDelayFeasible (it->second.curDc, it->second.curLvl))) {
		it->second.curLvl = UNPLACED_LVL;
	}
	modifiedChain = it->second;
	return true;
}

/*************************************************************************************************************************************************
* Returns the overall cpu cost at its current location.
**************************************************************************************************************************************************/
int ChainsMaster::calcNonMigCost () 
{
	
	int totNonMigCost = 0;
	for (auto const &chain : ChainsMaster::allChains) {	
		int16_t chainNonMigCost = chain.getCost ();
		if (MyConfig::mode==SYNC && chainNonMigCost == UNPLACED_COST) {
			snprintf (buf, bufSize, "ChainsMaster::calcNonMigCost: chain %d isn't placed yet", chain.id);
			MyConfig::printToLog (buf);
			return -1;
		}
		totNonMigCost += chainNonMigCost;
	}
	return totNonMigCost;
}


// Print the PoA of each currently-active user
void ChainsMaster::printAllChainsPoas () //(bool printSu=true, bool printleaf=false, bool printCurDatacenter=false)
{
	MyConfig::printToLog ("\nallChains\n*******************\n");
	MyConfig::printToLog ("format: (c,p), where c is the chain id, and p is the dcId of its poa\n");
	
	for (auto chain : ChainsMaster::allChains) {
		snprintf (buf, bufSize, "(%d,%d)", chain.id, chain.S_u[0]);
		MyConfig::printToLog (buf);
	}
}


/*************************************************************************************************************************************************
* Fill within numMigs the overall num of migrations in the last time period.
* If any chain is unplaced, return false. Else, return true.
**************************************************************************************************************************************************/
bool ChainsMaster::concludeTimePeriod (int &numMigs)
{

	Need to adapt this to allChains_, which is a map.
	numMigs = 0;
	
	for (auto chain : allChains) {
		if (chain.curLvl == UNPLACED_LVL) {
			snprintf (buf, bufSize, "\nERROR: ChainsMaster::concludeTimePeriod encountered the %d which is unplaced\n", chain.id);
			MyConfig::printToLog (buf); 
			return false;
		}
        if ((int)(chain.S_u).size()<(chain.curLvl-1)) {
            MyConfig::printToLog ("\nERROR: ChainsMaster::concludeTimePeriod encountered the following problematic chain:\n");
            MyConfig::printToLog (chain);
            return false;
        }
		if ( (chain.curDc != UNPLACED_DC) && (chain.curDc != chain.S_u[chain.curLvl]) ) { // Was the chain migrated?
			numMigs++;
		}
		Chain modifiedChain = chain; 
		modifiedChain.curDc = chain.S_u[chain.curLvl];
		allChains.erase  (chain);
		MyConfig::printToLog ("\nprinting modified chain here1\n");
		MyConfig::printToLog (modifiedChain);
		allChains.insert (modifiedChain);
	}
	return true;
}

void ChainsMaster::printAllChains ()
{
	for (auto chain : allChains) {
		snprintf (buf, bufSize, "chain %d, curDc=%d, curLvl=%d\n", chain.id, chain.curDc, chain.curLvl);
		MyConfig::printToLog (buf);
	}
}


/*************************************************************************************************************************************************
* Print to the log for every datacenter, the list of chains currently placed on it.
**************************************************************************************************************************************************/
void ChainsMaster::printAllDatacenters (int numDatacenters) 
{
	MyConfig::printToLog ("in ChainsMaster::printAllDatacenters\n");
	// gather the required data
	vector<ChainId_t> chainsPlacedOnDatacenter[numDatacenters]; //chainsPlacedOnDatacenter[dc] will hold a vector of the IDs of the chains currently placed on datacenter dc.
	for (const auto &chain : ChainsMaster::allChains) {
		DcId_t curDc = chain.curDc;
		if (curDc==UNPLACED_DC) {
			continue;
		}
		chainsPlacedOnDatacenter [curDc].push_back (chain.id);
		snprintf (buf, bufSize, "pushed chain % d on chainsPlacedOnDatacenter[%d]\n", chain.id, curDc);
		MyConfig::printToLog(buf);
	}
	
	// print the data
	for (DcId_t dcId(0); dcId<numDatacenters; dcId++) {
		snprintf (ChainsMaster::buf, ChainsMaster::bufSize, "DC %d, placed chains: ", dcId);
		MyConfig::printToLog(buf);
//		printBufToLog ();
		MyConfig::printToLog (chainsPlacedOnDatacenter[dcId]);
		MyConfig::printToLog ("\n");
	}
	
}

