#include <vector>
#include "Chain.h"

const vector<uint16_t> RT_Chain		 ::mu_u = {1, 5, 10};
const vector<uint16_t> Non_RT_Chain::mu_u = {1, 5, 10};

const uint8_t RT_Chain	  ::mu_u_len = 2;
const uint8_t Non_RT_Chain::mu_u_len = 2;

const vector <uint16_t> RT_Chain	  ::cpuCostAtLvl = MyConfig::scalarProdcut (RT_Chain		 ::mu_u, cpuCostAtLvl);
const vector <uint16_t> Non_RT_Chain::cpuCostAtLvl = MyConfig::scalarProdcut (Non_RT_Chain::mu_u, cpuCostAtLvl);

Chain::Chain (uint32_t id, vector <uint16_t> S_u) 
{
	this->id = id;
	this->S_u = S_u;
	curDatacenter = UNPLACED; 
};

RT_Chain::RT_Chain (uint32_t id, vector <uint16_t> S_u) {
  this->id        = id;
  this->S_u       = S_u;
  this->isRT_Chain = true;
	curDatacenter = UNPLACED; 
};

Non_RT_Chain::Non_RT_Chain (uint32_t id, vector <uint16_t> S_u) {
  this->id       = id;
  this->S_u       = S_u;
  this->isRT_Chain = false;
	curDatacenter = UNPLACED; 
};

uint16_t Chain::mu_u_len ()
{
	return (this->isRT_Chain)? RT_Chain::mu_u_len : Non_RT_Chain::mu_u_len;
}


uint16_t Chain::mu_u_at_lvl (uint8_t lvl)
{
	return (this->isRT_Chain)? RT_Chain::mu_u[lvl] : Non_RT_Chain::mu_u[lvl];
}


/* 
Find a chain within a given set of chains.
We assume that a chain is inequivocally identified by its id.
Inputs: 
- setOfChains: look for the chain within this set.
- id: of the requested chain.
- &foundChain: a reference, to which the found chain (if there exists) will be written.
Return value: true iff the requested chain was found.
*/
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

/*
Insert a chain to its correct order in the (ordered) vector of chains.
We currently use only RT, and we assume that the input vector is sorted. 
Hence, the chain should be inserted either to the head if it's a RT chain, of to the tail otherwise.
*/
void insertSorted (vector <Chain> &vec, const Chain c)
{
	if (c.isRT_Chain) {
		vec.insert (vec.begin(), c);
	}
	else {
		vec.push_back (c);
	}
}

/*
Rcvs 2 sorted vectors of chains. 
Put in the first vector (given by ref') a sorted vector, containing the union of the two input vectors. 
*/
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

