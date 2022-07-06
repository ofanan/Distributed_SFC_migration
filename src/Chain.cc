#include "Chain.h"

const vector<Cost_t> Chain::costOfCpuUnitAtLvl	 = {16, 8, 4, 2, 1};

// mu_u[i] is the amount of CPU required for placing a chain in a datacenter at level i

vector <Cost_t> RtChain		::costAtLvl; 
vector <Cost_t> NonRtChain::costAtLvl;

vector<Cpu_t> RtChain::		mu_u; 
vector<Cpu_t> NonRtChain::mu_u; 
Lvl_t RtChain	  ::mu_u_len; // = RtChain		 ::mu_u.size();
Lvl_t NonRtChain::mu_u_len; // = NonRtChain::mu_u.size();

Chain::Chain () 
{
	this->id 					= DUMMY_CHAIN_ID; 
	this->curLvl			= UNPLACED_LVL;
	this->curDc 			= UNPLACED_DC;
	this->potCpu			= UNPLACED_CPU;
	this->S_u 				= {};
	this->isRtChain 	= false;
};

Chain::Chain (ChainId_t id) 
{
	this->id 					= id; 
	this->curLvl 			= UNPLACED_LVL;
	this->curDc 			= UNPLACED_DC;
	this->potCpu			= UNPLACED_CPU;
	this->S_u 			 	= {};
	this->isRtChain 	= false;
};

Chain::Chain (ChainId_t id, vector <DcId_t> &S_u, Lvl_t curLvl) 
{
	this->id 					= id;
	this->S_u 				= S_u;
	this->curLvl 			= curLvl;
	this->curDc 			= UNPLACED_DC;
	this->potCpu			= UNPLACED_CPU;
	this->isRtChain 	= false;
};

Chain::Chain (const Chain &c) {
	this->id 					= c.id;
  this->S_u 				= c.S_u;
  this->curLvl			= c.curLvl;
  this->curDc 			= c.curDc;
	this->potCpu			= c.potCpu;
  this->isRtChain 	= c.isRtChain;
}

RtChain::RtChain (const RtChain &c) {
	this->id 					= c.id;
  this->S_u 				= c.S_u;
  this->curLvl			= c.curLvl;
  this->curDc 			= c.curDc;
	this->potCpu			= c.potCpu;
  this->isRtChain 	= true;
}

NonRtChain::NonRtChain (const NonRtChain &c) {
	this->id 					= c.id;
  this->S_u 				= c.S_u;
  this->curLvl			= c.curLvl;
  this->curDc 			= c.curDc;
	this->potCpu			= c.potCpu;
  this->isRtChain 	= false;
}

RtChain::RtChain (ChainId_t id, vector <DcId_t> &S_u) {
  this->id        	= id;
  this->S_u       	= S_u;
	this->curLvl 			= UNPLACED_LVL;
  this->curDc 			= UNPLACED_DC;
	this->potCpu			= UNPLACED_CPU;
  this->isRtChain 	= true;
};

NonRtChain::NonRtChain (ChainId_t id, vector <DcId_t> &S_u) {
  this->id       		= id;
  this->S_u      	 	= S_u;
	this->curLvl 			= UNPLACED_LVL;
  this->curDc 			= UNPLACED_DC;
	this->potCpu			= UNPLACED_CPU;
  this->isRtChain 	= false;
};


/*************************************************************************************************************************************************
* set this->potCpu according to the lvl of the pot-placing host, and the characteristic of the chain. 
* Currently unused, as the Dc set it themselves.
**************************************************************************************************************************************************/
//void Chain::setPotCpu (Lvl_t lvl) 
//{
//	this->potCpu = (this->isRtChain)? RtChain::mu_u[lvl] : NonRtChain::mu_u[lvl];
//}

void Chain::print (bool printS_u)
{
	int bufSize = 128;
	char buf[bufSize];
	if (printS_u) {
		snprintf (buf, bufSize, "chain %d: S_u=", id);
		MyConfig::printToLog (buf);
		MyConfig::printToLog (S_u);
	}
	else {
		snprintf (buf, bufSize, "%d, ", id);
		MyConfig::printToLog (buf);
	}
}

// returns the number of datacenters which are delay-feasible for this chain
Lvl_t Chain::mu_u_len () const
{
	return (this->isRtChain)? RtChain::mu_u_len : NonRtChain::mu_u_len;
}


// returns the mu_u (amount of cpu required by the chain) at a given level in the tree
Cpu_t Chain::mu_u_at_lvl (Lvl_t lvl) const
{
	return (this->isRtChain)? RtChain::mu_u[lvl] : NonRtChain::mu_u[lvl];
}

/*************************************************************************************************************************************************
* Given a set of chains and a poa, return all the chains in the set associated with this poa.
**************************************************************************************************************************************************/
vector<Chain> findChainsByPoa (unordered_set <Chain, ChainHash> setOfChains, DcId_t poa)
{
	vector<Chain> res;
	
	for (auto chain : setOfChains) {
		if (chain.S_u[0] == poa) {
			res.push_back (chain);
		}
	}
	return res;
}

/*************************************************************************************************************************************************
* Given a chain id, if that chain is found in the given set - erase it from the set.
* Returns true iff the requested chain was found (and erased) from the set.
**************************************************************************************************************************************************/
bool eraseChainFromSet (UnorderedSetOfChains &setOfChains, ChainId_t chainId)
{
	Chain dummy (chainId);
	auto search = setOfChains.find (dummy);

	if (search==setOfChains.end()) {
		return false;
	}
	setOfChains.erase(search);
	return true;
}

/*************************************************************************************************************************************************
* Given a chain id, find the respective chain within a given set of chains.
* The chain is written to foundChain.
* Output: true iff the requested chain was found.
**************************************************************************************************************************************************/
bool findChainInSet (const unordered_set <Chain, ChainHash> setOfChains, ChainId_t chainId, Chain &c)
{
	Chain dummy (chainId);
	auto search = setOfChains.find (dummy);

	if (search==setOfChains.end()) {
		return false;
	}
	else {
		c = *search;
		return true;
	}
}

//void Chain::setNetType (const unsigned int netType)
//{
//	Chain::netType = netType;
//}

//// returns the id of the datacenter currently hosting "this"; or UNPLACED_DC, if this chain isn't placed
//DcId_t Chain::getCurDatacenter () const 
//{
//	return (curLvl==UNPLACED_LVL)? UNPLACED_DC : S_u[curLvl];
//} 

/*************************************************************************************************************************************************
* Returns the non-mig' cost at the current place. If the chain isn't placed, the function returns UNPLACED_COST.
**************************************************************************************************************************************************/
Cost_t Chain::getCost () const
{
	return (curLvl==UNPLACED_LVL)? UNPLACED_COST : ((isRtChain)? RtChain::costAtLvl[curLvl] : NonRtChain::costAtLvl[curLvl]);
}

/*************************************************************************************************************************************************
* Returns the non-mig' cost if placed at a certain level.. If the chain isn't placed, the function returns UNPLACED_COST.
**************************************************************************************************************************************************/
Cost_t Chain::getCostAtLvl (const Lvl_t lvl) const
{
	return (isRtChain)? RtChain::costAtLvl[lvl] : NonRtChain::costAtLvl[lvl];
}

/*************************************************************************************************************************************************
* Return the current cpu consumption of the chain if it's already placed; UNPLACED_CPU otherwise
**************************************************************************************************************************************************/
Cpu_t Chain::getCpu () const
{
	if (curLvl==UNPLACED_LVL) {
		return UNPLACED_CPU;
	}
	else {
	  return (isRtChain)? RtChain::mu_u[curLvl] : NonRtChain::mu_u[curLvl];
	}
}

/*************************************************************************************************************************************************
Insert a chain to its correct order in the (ordered) vector of chains.
We currently use only RT, and we assume that the input vector is sorted. 
Hence, the chain should be inserted either to the head if it's a RT chain, of to the tail otherwise.
Currently unused, bacuase it's easier to insert chains wo sorting, and sort only when needed.
**************************************************************************************************************************************************/
void insertSorted (vector <Chain> &vec, const Chain &c)
{
	if (c.isRtChain) {
		vec.insert (vec.begin(), c);
	}
	else {
		vec.push_back (c);
	}
}

/*************************************************************************************************************************************************
insert a chain to its correct location in a sorted list.
If the chain (recognized equivocally by its id) is already found in the list, the old occurance in the list is deleted.
**************************************************************************************************************************************************/
bool insertChainToList (list <Chain> &sortedList, const Chain &chain)
{

	if (chain.potCpu==UNPLACED_CPU) {
		int bufSize = 128;
		char buf[bufSize];	
		snprintf (buf, bufSize, "\error: insertChainToList was called with chain %d for which postCpu==UNPLACED_CPU", chain.id);
		MyConfig::printToLog (buf);
		return false;
	}
	
	
	// delete the old occurance of that chain in the list, if exists
	for (auto chainPtr = sortedList.begin(); chainPtr!=sortedList.end(); ) {
		if (chainPtr->id == chain.id) {
			sortedList.erase (chainPtr);
			break;
		}
		chainPtr++;
	}

	// insert the chain
  sortedList.insert(sortedList.begin(), chain);
  return true;
}


