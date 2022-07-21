#include "ChainsMaster.h"

unordered_map <ChainId_t, Chain> ChainsMaster::allChains; // All the currently active chains. 
int ChainsMaster::numInstantMigs; // Instantaneous num of migs, including those happen and later "cancelled" by a reshuffle in the same period.
const int ChainsMaster::bufSize;
char ChainsMaster::buf[bufSize];

/*************************************************************************************************************************************************
* Erases chains (given by a vector of chainIds) from allChains.
* Returns false if one of the chains to erase wasn't found in allChains, else true.
**************************************************************************************************************************************************/
bool ChainsMaster::eraseChains (vector <ChainId_t> &vec)
{
	for (auto chainId : vec) {
		auto it = ChainsMaster::allChains.find(chainId);
		if (it == ChainsMaster::allChains.end()) { 
			snprintf (buf, bufSize, "\nERROR: ChainsMaster::eraseChains didn't find chain %d", chainId);
			MyConfig::printToLog (buf);
			return false;
		}
		ChainsMaster::allChains.erase (it);
	}
	return true;
}

/*************************************************************************************************************************************************
* Block a chain, given its id
* Returns false if the chains to block wasn't found in allChains, else true.
**************************************************************************************************************************************************/
bool ChainsMaster::blockChain  (ChainId_t chainId) 
{
	auto it = ChainsMaster::allChains.find(chainId);
	if (it == ChainsMaster::allChains.end()) { 
		snprintf (buf, bufSize, "\nERROR: ChainsMaster::eraseChain didn't find chain %d", chainId);
		MyConfig::printToLog (buf);
		return false;
	}
	it->second.isBlocked = true;
	if (MyConfig::DEBUG_LVL > 0 && (it->second.curLvl != UNPLACED_LVL || it-> second.curDc != UNPLACED_DC))  {
	  	snprintf (buf, bufSize, "trace Time=%f, chain %d was blocked whlie having curLvl=%d and curDc=%d", MyConfig::traceTime, chainId, it->second.curLvl, it->second.curDc);
	  	MyConfig::printToLog (buf);
	  	return false;
	}
	if (MyConfig::LOG_LVL==VERY_DETAILED_LOG) {
		snprintf (buf, bufSize, "\nblocked chain %d", chainId); 
		MyConfig::printToLog(buf); 
	}	
	MyConfig::overallNumBlockedUsrs++;
	return true;
}

/*************************************************************************************************************************************************
* Inserts the chain, given its id, into the by-ref argument chain.
* Returns false the ChainId_t was already found in allChains.
**************************************************************************************************************************************************/
bool ChainsMaster::insert (ChainId_t chainId, Chain chain)
{
	if (MyConfig::DEBUG_LVL>1) {
		auto it = ChainsMaster::allChains.find(chainId);
		if (it == ChainsMaster::allChains.end()) { 
			return false;
		}
	}
	ChainsMaster::allChains.insert ({chainId, chain}); 
	return true;
}


/*************************************************************************************************************************************************
* Finds the chain, given its id, into the by-ref argument chain.
* Returns true iff the ChainId_t is found in allChains.
**************************************************************************************************************************************************/
bool ChainsMaster::findChain (ChainId_t chainId, Chain &chain)
{
	auto it = ChainsMaster::allChains.find(chainId);
	if (it == ChainsMaster::allChains.end()) { 
		return false;
  }
  chain = it->second;
  return true;
}


/*************************************************************************************************************************************************
* Get the cur Dc, in which a chain is currently placed. If the chain isn't placed, the curDc is UNPLACED_DC.
* Assigns the value into dcId, given by ref'.
* Returns true iff the ChainId_t is found in allChains.
**************************************************************************************************************************************************/
bool ChainsMaster::getChainCurDc (ChainId_t chainId, DcId_t &dcId)
{
	auto it = ChainsMaster::allChains.find(chainId);
	if (it == ChainsMaster::allChains.end()) { 
		return false;
  }
  dcId = it->second.curDc;
  return true;
}


/*************************************************************************************************************************************************
* Displace all chains, by setting their curLvl to UNPLACED_LVL.
**************************************************************************************************************************************************/
void ChainsMaster::displaceAllChains ()
{
	for (auto it=ChainsMaster::allChains.begin(); it!=allChains.end(); it++) {
		it->second.curLvl = UNPLACED_LVL;
	}
}

/*************************************************************************************************************************************************
* Given a list of chain ids, update the curLvl field of the all the chains in the list to the given newLvl.
* Output: true iff all the chains in the list were found, and their levels were updated to newLvl.
* The function also increments the number of inst' migrations (numInstantMigs).
**************************************************************************************************************************************************/
bool ChainsMaster::modifyLvl (unordered_set <ChainId_t> &listOfChainIds, Lvl_t newLvl)
{
	for (ChainId_t chainId : listOfChainIds) {
		auto it = ChainsMaster::allChains.find(chainId);
		if (it == ChainsMaster::allChains.end()) { 
			snprintf (buf, bufSize, "\nerror: ChainsMaster::modifyLvl didn't find chain %d", chainId);
			MyConfig::printToLog (buf);
			return false;
		}
		it->second.curLvl = newLvl;
		it->second.potCpu = UNPLACED_CPU; // as the chain is now really placed, reset its "potential Cpu" value.
		ChainsMaster::numInstantMigs++; // assume that every change in the lvl implies an "instantaneous migration" (several inst' mig' may happen per period).	
	}
  return true;
}

/*************************************************************************************************************************************************
* Given a chain id, update the curLvl field of the respective chain to the given newLvl.
* Output: true iff the requested chain was found, and its level was updated to newLvl.
* The function also increments the number of inst' migrations (numInstantMigs).
**************************************************************************************************************************************************/
bool ChainsMaster::modifyLvl (ChainId_t chainId, Lvl_t newLvl)
{
	auto it = ChainsMaster::allChains.find(chainId);
	if (it == ChainsMaster::allChains.end()) { 
		return false;
  }
  it->second.curLvl = newLvl;
  it->second.potCpu = UNPLACED_CPU; // as the chain is now really placed, reset its "potential Cpu" value.
	ChainsMaster::numInstantMigs++; // assume that every change in the lvl implies an "instantaneous migration" (several inst' mig' may happen per period).
  return true;
}

/*************************************************************************************************************************************************
* Given a chain id, fill into the isBlocked input (given by ref') true iff this chain is blocked.
* Output: true iff the requested chain was found.
**************************************************************************************************************************************************/
bool ChainsMaster::checkIfBlocked (ChainId_t chainId, bool &isBlocked)
{
	auto it = ChainsMaster::allChains.find(chainId);
	if (it == ChainsMaster::allChains.end()) { 
		return false;
  }
  isBlocked = it->second.isBlocked;
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

	auto it = ChainsMaster::allChains.find(chainId);
	if (it == ChainsMaster::allChains.end()) { 
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
* Returns the overall (cpu cost + link cost) of all chains.
* Assuming that ChainsMaster::allChains includes the updated information w.r.t. where each chain is currently placed.
**************************************************************************************************************************************************/
int ChainsMaster::calcNonMigCost () 
{
	int totNonMigCost = 0;
	for (auto it=ChainsMaster::allChains.begin(); it!=allChains.end(); it++) {
		if (it->second.isBlocked) {
			continue;
		}
		int16_t chainNonMigCost = it->second.getCost ();
		if (MyConfig::mode==Sync && chainNonMigCost == UNPLACED_COST) {
			snprintf (buf, bufSize, "ChainsMaster::calcNonMigCost: chain %d isn't placed yet", it->second.id);
			MyConfig::printToLog (buf);
			return -1;
		}
		totNonMigCost += chainNonMigCost;
	}
	return totNonMigCost;
}



/*************************************************************************************************************************************************
* Print the PoA of each currently-active user.
**************************************************************************************************************************************************/
void ChainsMaster::printAllChainsPoas () //(bool printSu=true, bool printleaf=false, bool printCurDatacenter=false)
{
	MyConfig::printToLog ("\nallChains\n*******************\n");
	MyConfig::printToLog ("format: (c,p), where c is the chain id, and p is the dcId of its poa\n");
	
	for (auto it=ChainsMaster::allChains.begin(); it!=allChains.end(); it++) {
		snprintf (buf, bufSize, "(%d,%d)", it->second.id, it->second.S_u[0]);
		MyConfig::printToLog (buf);
	}
}


/*************************************************************************************************************************************************
Writes into numMigs the overall num of migrations in the last time period.
Update for each chain c: c.curDc to be its currently hosting datacenter.
- If any chain is unplaced, return 1, and set errChainId to the id of this erroneous chain.
- Else, if any chain is placed incorrectly, return 2, and set errChainId to the id of this erroneous chain.
- Else, return 0.
**************************************************************************************************************************************************/
int ChainsMaster::concludeTimePeriod (int &numMigs, int &curNumBlockedUsrs, ChainId_t &errChainId)
{
	numMigs = 0;
	curNumBlockedUsrs = 0;
	for (auto it=ChainsMaster::allChains.begin(); it!=allChains.end(); it++) {
		if (it->second.isBlocked) {
			curNumBlockedUsrs++;
			continue;
		}
		if (MyConfig::DEBUG_LVL>0 && it->second.curLvl == UNPLACED_LVL) {
			errChainId = it->second.id;
			snprintf (buf, bufSize, "\nerror: ChainsMaster::concludeTimePeriod encountered chain %d which is unplaced\n", it->second.id);
			MyConfig::printToLog (buf); 
			return 1;
		}
    if ((int)(it->second.S_u).size()<(it->second.curLvl-1)) {
				errChainId = it->second.id;
        MyConfig::printToLog ("\nerror: ChainsMaster::concludeTimePeriod encountered the following problematic chain:\n");
        MyConfig::printToLog (it->second);
        return 2;
    }
		if ( (it->second.curDc != UNPLACED_DC) && (it->second.curDc != it->second.S_u[it->second.curLvl]) ) { // Did that chain migrate?
			numMigs++;
			if (MyConfig::LOG_LVL >= DETAILED_LOG) {
				snprintf (buf, bufSize, "\nchain %d migrated from %d to %d", it->second.id, it->second.curDc, it->second.S_u[it->second.curLvl]);
				MyConfig::printToLog (buf);
			}
		}
		it->second.curDc = it->second.S_u[it->second.curLvl];
		it->second.potCpu = UNPLACED_CPU; // reset; will be set again only when this chain is pot-placed
	}	
	return 0;
}

/*************************************************************************************************************************************************
* Writes into numMigs the overall num of migrations in the last time period.
* Update for each chain c: c.curDc to be its currently hosting datacenter.
* If any chain is unplaced, return false. Else, return true.
**************************************************************************************************************************************************/
void ChainsMaster::printAllChains ()
{
	for (auto it=ChainsMaster::allChains.begin(); it!=allChains.end(); it++) {
		snprintf (buf, bufSize, "chain %d, curDc=%d, curLvl=%d\n", it->second.id, it->second.curDc, it->second.curLvl);
		MyConfig::printToLog (buf);
	}
}


/*************************************************************************************************************************************************
* Print to the log for every datacenter, the list of chains currently placed on it.
**************************************************************************************************************************************************/
void ChainsMaster::printAllDatacenters (int numDatacenters) 
{

	// gather the required data
	vector<ChainId_t> chainsPlacedOnDatacenter[numDatacenters]; //chainsPlacedOnDatacenter[dc] will hold a vector of the IDs of the chains currently placed on datacenter dc.
	for (auto it=ChainsMaster::allChains.begin(); it!=allChains.end(); it++) {
		DcId_t curDc = it->second.curDc;
		if (curDc==UNPLACED_DC) {
			continue;
		}
		chainsPlacedOnDatacenter [curDc].push_back (it->first);
	}
	
	// print the data
	for (DcId_t dcId(0); dcId<numDatacenters; dcId++) {
		snprintf (ChainsMaster::buf, ChainsMaster::bufSize, "DC %d, placed chains: ", dcId);
		MyConfig::printToLog(buf);
		MyConfig::printToLog (chainsPlacedOnDatacenter[dcId]);
		MyConfig::printToLog ("\n");
	}	
}

/*************************************************************************************************************************************************
* clear the db
**************************************************************************************************************************************************/
void ChainsMaster::clear ()
{
	ChainsMaster::allChains.clear ();
} 

