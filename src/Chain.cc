#include <vector>
#include "Chain.h"
#include "MyConfig.h"

const vector<uint16_t> Chain::costOfCpuUnitAtLvl	 = {16, 8, 4, 2, 1};

const vector<uint16_t> RT_Chain		 ::mu_u = {1, 1, 1};
const vector<uint16_t> Non_RT_Chain::mu_u = {1, 1, 1};

const uint8_t RT_Chain	  ::mu_u_len = RT_Chain		 ::mu_u.size();
const uint8_t Non_RT_Chain::mu_u_len = Non_RT_Chain::mu_u.size();


const vector <uint16_t> RT_Chain	  ::cpuCostAtLvl = MyConfig::scalarProdcut (RT_Chain::mu_u, 	  Chain::costOfCpuUnitAtLvl); 
const vector <uint16_t> Non_RT_Chain::cpuCostAtLvl = MyConfig::scalarProdcut (Non_RT_Chain::mu_u, Chain::costOfCpuUnitAtLvl); 

Chain::Chain (uint32_t id, vector <uint16_t> S_u) 
{
	this->id = id;
	this->S_u = S_u;
	curLvl = UNPLACED_;
};

Chain::Chain (const Chain &c) {
	this->id 					= c.id;
  this->S_u 				= c.S_u;
  this->isRT_Chain 	= c.isRT_Chain;
  this->curLvl			= c.curLvl;
}


RT_Chain::RT_Chain (uint32_t id, vector <uint16_t> S_u) {
  this->id        	= id;
  this->S_u       	= S_u;
  this->isRT_Chain 	= true;
};

Non_RT_Chain::Non_RT_Chain (uint32_t id, vector <uint16_t> S_u) {
  this->id       		= id;
  this->S_u      	 	= S_u;
  this->isRT_Chain 	= false;
};

void Chain::print (bool printS_u)
{
	uint8_t bufSize = 128;
	char buf[bufSize];
	snprintf (buf, bufSize, "chain %d", id);
	MyConfig::printToLog (buf);
	if (printS_u) {
		MyConfig::printToLog (": S_u=");
		MyConfig::printToLog (S_u);
	}
	else {
		MyConfig::printToLog ("\n");	
	}
}

uint16_t Chain::mu_u_len () const
{
	return (this->isRT_Chain)? RT_Chain::mu_u_len : Non_RT_Chain::mu_u_len;
}


uint16_t Chain::mu_u_at_lvl (uint8_t lvl) const
{
	return (this->isRT_Chain)? RT_Chain::mu_u[lvl] : Non_RT_Chain::mu_u[lvl];
}


/*************************************************************************************************************************************************
Find a chain within a given set of chains.
We assume that a chain is inequivocally identified by its id.
Inputs: 
- setOfChains: look for the chain within this set.
- id: of the requested chain.
- &foundChain: a reference, to which the found chain (if there exists) will be written.
Return value: true iff the requested chain was found.

**************************************************************************************************************************************************/
bool findChainInSet (unordered_set <Chain, ChainHash> setOfChains, uint32_t chainId, Chain &c)
{
	Chain dummy (chainId, {});
	auto search = setOfChains.find (dummy);

	if (search==setOfChains.end()) {
		return false;
	}
	else {
		c = *search;
		return true;
	}
}

// returns the id of the datacenter currently hosting "this"; or UNPLACED, if this chain isn't placed
int16_t Chain::getCurDatacenter () const 
{
	return (curLvl==UNPLACED_)? UNPLACED : S_u[curLvl];
} 

// returns the cpu cost of "this" chain when placed on a datacenter at a certain 
uint16_t Chain::getCpuCost () const
{
	return (isRT_Chain)? RT_Chain::cpuCostAtLvl[curLvl] : Non_RT_Chain::cpuCostAtLvl[curLvl];
}

// return the current cpu consumption of the chain if it's already placed; UNPLACED_ otherwise
inline uint16_t Chain::getCpu () const
{
	if (curLvl==UNPLACED_) {
		return -1;
	}
	else {
	  return (isRT_Chain)? RT_Chain::mu_u[curLvl] : Non_RT_Chain::mu_u[curLvl];
	}
}

/*************************************************************************************************************************************************
Insert a chain to its correct order in the (ordered) vector of chains.
We currently use only RT, and we assume that the input vector is sorted. 
Hence, the chain should be inserted either to the head if it's a RT chain, of to the tail otherwise.
**************************************************************************************************************************************************/
void insertSorted (vector <Chain> &vec, const Chain c)
{
	if (c.isRT_Chain) {
		vec.insert (vec.begin(), c);
	}
	else {
		vec.push_back (c);
	}
}

/*************************************************************************************************************************************************
Rcvs 2 sorted vectors of chains. 
Put in the first vector (given by ref') a sorted vector, containing the union of the two input vectors. 
Currently unused.
**************************************************************************************************************************************************/
void MergeSort (vector <Chain> &vec, const vector <Chain> vec2union)
{
	for (auto const &chain : vec) {
		if (chain.isRT_Chain) {
			vec.insert (vec.begin(), chain);
		}
		else {
			vec.push_back (chain);
		}
	}
}

