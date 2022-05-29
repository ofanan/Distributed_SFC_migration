#include <vector>
#include <type_traits>
#include "Chain.h"

const vector<uint16_t> Chain::costOfCpuUnitAtLvl	 = {16, 8, 4, 2, 1};

const vector<uint16_t> RT_Chain		 ::mu_u = {1, 1};
const vector<uint16_t> Non_RT_Chain::mu_u = {1, 1};

const uint8_t RT_Chain	  ::mu_u_len = RT_Chain		 ::mu_u.size();
const uint8_t Non_RT_Chain::mu_u_len = Non_RT_Chain::mu_u.size();


const vector <uint16_t> RT_Chain	  ::cpuCostAtLvl = MyConfig::scalarProdcut (RT_Chain::mu_u, 	  Chain::costOfCpuUnitAtLvl); 
const vector <uint16_t> Non_RT_Chain::cpuCostAtLvl = MyConfig::scalarProdcut (Non_RT_Chain::mu_u, Chain::costOfCpuUnitAtLvl); 

Chain::Chain () 
{
	this->id = DUMMY; // Should change id to be int32_6
	this->curLvl = UNPLACED_;
	this->S_u = {};
	this->isRT_Chain = false;
};

Chain::Chain (ChainId_t id, vector <uint16_t> S_u) 
{
	this->id = id;
	this->S_u = S_u;
	this->curLvl = UNPLACED_;
	this->isRT_Chain = false;
};

Chain::Chain (const Chain &c) {
	this->id 					= c.id;
  this->S_u 				= c.S_u;
  this->curLvl			= c.curLvl;
  this->isRT_Chain 	= c.isRT_Chain;
}


RT_Chain::RT_Chain (ChainId_t id, vector <uint16_t> S_u) {
  this->id        	= id;
  this->S_u       	= S_u;
	this->curLvl = UNPLACED_;
  this->isRT_Chain 	= true;
};

Non_RT_Chain::Non_RT_Chain (ChainId_t id, vector <uint16_t> S_u) {
  this->id       		= id;
  this->S_u      	 	= S_u;
	this->curLvl = UNPLACED_;
  this->isRT_Chain 	= false;
};

void Chain::print (bool printS_u)
{
	uint8_t bufSize = 128;
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
uint16_t Chain::mu_u_len () const
{
	return (this->isRT_Chain)? RT_Chain::mu_u_len : Non_RT_Chain::mu_u_len;
}


// returns the mu_u (amount of cpu required by the chain) at a given level in the tree
uint16_t Chain::mu_u_at_lvl (uint8_t lvl) const
{
	return (this->isRT_Chain)? RT_Chain::mu_u[lvl] : Non_RT_Chain::mu_u[lvl];
}

//// returns true iff the given datacenter id is delay-feasible for this chain (namely, appears in its S_u)
//bool Chain::isDelayFeasible (uint16_t dcId) const 
//{
//	for (auto const dc : S_u) {
//		if (dc==dcId) { // the suggested datacenter appears in my vector of delay-feasible datacenters
//			return true;
//		}
//	}
//	return false;
//}


/*************************************************************************************************************************************************
* Given a set of chains and a poa, return all the chains in the set associated with this poa.
**************************************************************************************************************************************************/
vector<Chain> findChainsByPoa (unordered_set <Chain, ChainHash> setOfChains, uint16_t poa)
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
bool eraseChainFromSet (UnorderedSetOfChains &setOfChains, uint16_t chainId)
{
	Chain dummy (chainId, {});
	auto search = setOfChains.find (dummy);

	if (search==setOfChains.end()) {
		return false;
	}
	else {
		setOfChains.erase(search);
		return true;
	}
}

/*************************************************************************************************************************************************
* Given a chain id, finding the respective chain within a given set of chains.
* The chain is written to foundChain.
* Output: true iff the requested chain was found.
**************************************************************************************************************************************************/
bool findChainInSet (unordered_set <Chain, ChainHash> setOfChains, ChainId_t chainId, Chain &c)
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

// returns UNPLACED if the chain isn't placed; and the cpu cost at the current place
uint16_t Chain::getCpuCost () const
{
	return (curLvl==UNPLACED_)? UNPLACED : ((isRT_Chain)? RT_Chain::cpuCostAtLvl[curLvl] : Non_RT_Chain::cpuCostAtLvl[curLvl]);
}

// return the current cpu consumption of the chain if it's already placed; UNPLACED_ otherwise
uint16_t Chain::getCpu () const
{
	if (curLvl==UNPLACED_) {
		return UNPLACED;
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

bool UsesMoreCpu (const Chain& lhs, const Chain& rhs) {
        return lhs.getCpu () >= rhs.getCpu ();
}

//struct sortTwoChainsByCpuUsage {
//        bool operator () (const Chain& lhs, const Chain& rhs) const {
//                return lhs.getCpu () >= rhs.getCpu ();
//        
///*              if (lhs.curLvl==-1 || rhs.curLvl==-1) { // if either lhs, or rhs, is unplaced, arbitrarily return false*/
///*                      return false;*/
///*              }*/
///*        return ((lhs.isRT_Chain)? RT_Chain::mu_u[lhs.curLvl] : Non_RT_Chain::mu_u[lhs.curLvl]) <*/
///*                               ((rhs.isRT_Chain)? RT_Chain::mu_u[rhs.curLvl] : Non_RT_Chain::mu_u[rhs.curLvl]);*/
//        } 
//};

//typedef unordered_set <Chain, ChainHash>                 UnorderedSetOfChains;
//typedef set <Chain, sortTwoChainsByCpuUsage> SetOfChainsOrderedByCpuUsage; 


