#include "Chain.h"

const vector<Cost_t> Chain::costOfCpuUnitAtLvl	 = {16, 8, 4, 2, 1};

// mu_u[i] is the amount of CPU required for placing a chain in a datacenter at level i
const vector<Cpu_t> RT_Chain		::mu_u = {1, 1}; 
const vector<Cpu_t> Non_RT_Chain::mu_u = {1, 1, 1};

const Lvl_t RT_Chain	  ::mu_u_len = RT_Chain		 ::mu_u.size();
const Lvl_t Non_RT_Chain::mu_u_len = Non_RT_Chain::mu_u.size();

// cpuCostAtLvl[i] is the cost of placing a chain on a datacenter at level i
const vector <Cost_t> RT_Chain	  ::cpuCostAtLvl = MyConfig::scalarProdcut (RT_Chain::mu_u, 	  Chain::costOfCpuUnitAtLvl); 
const vector <Cost_t> Non_RT_Chain::cpuCostAtLvl = MyConfig::scalarProdcut (Non_RT_Chain::mu_u, Chain::costOfCpuUnitAtLvl); 

Chain::Chain () 
{
	this->id = DUMMY_CHAIN_ID; 
	this->curLvl = UNPLACED_LVL;
	this->S_u = {};
	this->isRT_Chain = false;
};

Chain::Chain (ChainId_t id, vector <DcId_t> S_u, Lvl_t curLvl) 
{
	this->id = id;
	this->S_u = S_u;
	this->curLvl = curLvl;
	this->isRT_Chain = false;
};

Chain::Chain (const Chain &c) {
	this->id 					= c.id;
  this->S_u 				= c.S_u;
  this->curLvl			= c.curLvl;
  this->isRT_Chain 	= c.isRT_Chain;
}

RT_Chain::RT_Chain (const RT_Chain &c) {
	this->id 					= c.id;
  this->S_u 				= c.S_u;
  this->curLvl			= c.curLvl;
//  this->isRT_Chain 	= true;
}

Non_RT_Chain::Non_RT_Chain (const Non_RT_Chain &c) {
	this->id 					= c.id;
  this->S_u 				= c.S_u;
  this->curLvl			= c.curLvl;
//  this->isRT_Chain 	= false;
}

RT_Chain::RT_Chain (ChainId_t id, vector <DcId_t> S_u) {
  this->id        	= id;
  this->S_u       	= S_u;
	this->curLvl = UNPLACED_LVL;
//  this->isRT_Chain 	= true;
};

Non_RT_Chain::Non_RT_Chain (ChainId_t id, vector <DcId_t> S_u) {
  this->id       		= id;
  this->S_u      	 	= S_u;
	this->curLvl = UNPLACED_LVL;
//  this->isRT_Chain 	= false;
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

//// returns the number of datacenters which are delay-feasible for this chain
//Lvl_t RT_Chain::mu_u_len () const
//{
//	return RT_Chain::mu_u_len;
//}

//// returns the number of datacenters which are delay-feasible for this chain
//Lvl_t Non_RT_Chain::mu_u_len () const
//{
//	return Non_RT_Chain::mu_u_len;
//}

// returns the mu_u (amount of cpu required by the chain) at a given level in the tree
Cpu_t Non_RT_Chain::mu_u_at_lvl (Lvl_t lvl) const
{
	return Non_RT_Chain::mu_u[lvl];
}

// returns the mu_u (amount of cpu required by the chain) at a given level in the tree
Cpu_t RT_Chain::mu_u_at_lvl (Lvl_t lvl) const
{
	return RT_Chain::mu_u[lvl];
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
	Chain dummy (chainId, {});
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

// returns the id of the datacenter currently hosting "this"; or UNPLACED_DC, if this chain isn't placed
DcId_t Chain::getCurDatacenter () const 
{
	return (curLvl==UNPLACED_LVL)? UNPLACED_DC : S_u[curLvl];
} 

// returns UNPLACED_COST if the chain isn't placed; and the cpu cost at the current place
Cost_t RT_Chain::getCpuCost () const
{
	return RT_Chain::cpuCostAtLvl[curLvl];
}

// returns UNPLACED_COST if the chain isn't placed; and the cpu cost at the current place
Cost_t Non_RT_Chain::getCpuCost () const
{
	return Non_RT_Chain::cpuCostAtLvl[curLvl];
}

// return the current cpu consumption of the chain if it's already placed; UNPLACED_CPU otherwise
Cpu_t RT_Chain::getCpu () const
{
	if (curLvl==UNPLACED_LVL) {
		return UNPLACED_CPU;
	}
	return RT_Chain::mu_u[curLvl];
}

// return the current cpu consumption of the chain if it's already placed; UNPLACED_CPU otherwise
Cpu_t Non_RT_Chain::getCpu () const
{
	if (curLvl==UNPLACED_LVL) {
		return UNPLACED_CPU;
	}
	return Non_RT_Chain::mu_u[curLvl];
}

/*************************************************************************************************************************************************
Insert a chain to its correct order in the (ordered) vector of chains.
We currently use only RT, and we assume that the input vector is sorted. 
Hence, the chain should be inserted either to the head if it's a RT chain, of to the tail otherwise.
**************************************************************************************************************************************************/
void insertSorted (vector <Chain> &vec, Chain c)
{
	if (c.isRT_Chain) {
		vec.insert (vec.begin(), c);
	}
	else {
		vec.push_back (c);
	}
}


/*************************************************************************************************************************************************
Insert a chain to its correct order in the (ordered) vector of chains.
We currently use only RT, and we assume that the input vector is sorted. 
Hence, the chain should be inserted either to the head if it's a RT chain, of to the tail otherwise.
**************************************************************************************************************************************************/
void insertSorted (vector <Chain> &vec, const RT_Chain c)
{
	vec.insert (vec.begin(), c);
}

void insertSorted (vector <Chain> &vec, const Non_RT_Chain c)
{
	vec.push_back (c);
}

// the compare function used by pushUpList: sort two chains in a decreasing order of the cpu they current use.
inline bool CompareChainsByDecCpuUsage (const Chain & lhs, const Chain & rhs) {
	Cpu_t lhsCpu = lhs.getCpu ();
	Cpu_t rhsCpu = rhs.getCpu ();
	if (lhsCpu==UNPLACED_CPU ||  rhsCpu==UNPLACED_CPU) {  // break ties
		return true;
	}
	return lhsCpu > rhsCpu;
}

/*************************************************************************************************************************************************
insert a chain to its correct location in a sorted list.
If the chain (recognized equivocally by its id) is already found in the list, the old occurance in the list is deleted.
**************************************************************************************************************************************************/
void insertSorted (list <Chain> &sortedList, const Chain chain)
{

	for (auto chainPtr = sortedList.begin(); chainPtr!=sortedList.end(); ) {
		if (chainPtr->id == chain.id) {
			sortedList.erase (chainPtr);
			break;
		}
		chainPtr++;
	}

	auto begin 	= sortedList.begin();
  auto end 		= sortedList.end();
  while ((begin != end) && CompareChainsByDecCpuUsage (*begin, chain)) { // skip all chains in the list which use more cpu than me
  	begin++;
  }
  sortedList.insert(begin, chain);
}


///*************************************************************************************************************************************************
//Rcvs 2 sorted vectors of chains. 
//Put in the first vector (given by ref') a sorted vector, containing the union of the two input vectors. 
//Currently unused.
//**************************************************************************************************************************************************/
//void MergeSort (vector <Chain> &vec, const vector <Chain> vec2union)
//{
//	for (auto const &chain : vec) {
//		if (chain.isRT_Chain) {
//			vec.insert (vec.begin(), chain);
//		}
//		else {
//			vec.push_back (chain);
//		}
//	}
//}

