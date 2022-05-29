#ifndef CHAIN_H
#define CHAIN_H
#include <stdio.h>
#include <stdint.h>
#include <omnetpp.h>

#include <vector>
#include <set>
#include <algorithm>
#include <unordered_set>

#include "MyConfig.h"
using namespace std;

typedef int32_t ChainId_t;


class Chain
{
  public:
    ChainId_t id;
    vector <uint16_t> S_u;         // List of delay-feasible datacenters for this chain
    bool isRT_Chain;
		const static vector<uint16_t> costOfCpuUnitAtLvl; 
		int8_t curLvl;        // Level of the datacenter currently hosting me 
		//    bool isNew;        // When true, this chain is new (not currently scheduled to any datacenter). 

		// C'tors
    Chain ();
		Chain (const Chain &c);
    Chain (ChainId_t id, vector <uint16_t> S_u);
            
    bool operator== (const Chain &right) const {
      return (this->id == right.id);
    }
	
		void print (bool printS_u = true);	
	
		// Getters
		int16_t  getCurDatacenter () const; // returns the id of the datacenter currently hosting this; or UNPLACED, if this chain isn't placed
    uint16_t getCpuCost () const;
    uint16_t getCpu     () const; 
    
		/* 
		We order chain by non-increasing order of |S_u|, namely how high they can be located in the tree; iteratively breaking ties by decreasing mu_u[l] for each level \ell, namely, the amount of CPU 
		rsrcs required for locating the chain at a certain lvl.
		However, in our current sim settings, all of this is degenerated to the fast and efficient rule that an RT chain has higher priority (==smaller #), over a non-RT chain.
    bool operator< (const Chain &right) const {
      return (this->isRT_Chain && !(right.isRT_Chain));
    }
		*/		
    uint16_t mu_u_at_lvl (uint8_t lvl) const; // returns the amount of cpu required for placing this chain at level lvl
    uint16_t mu_u_len () const;
/*    bool isDelayFeasible (uint16_t dcId) const;*/
};

class RT_Chain : public Chain
{
public:
  static const vector<uint16_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
  static const uint8_t mu_u_len;
	static const vector<uint16_t> cpuCostAtLvl; // cpuCostAtLvl[i] will hold the cost of placing an RT chain on a DC in level i
  RT_Chain (ChainId_t id, vector <uint16_t> S_u);
};

class Non_RT_Chain: public Chain
{
  public:
	  static const vector<uint16_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
	  static const uint8_t  mu_u_len;
		static const vector<uint16_t> cpuCostAtLvl; // cpuCostAtLvl[i] will hold the cost of placing a non- RT chain on a DC in level i
    Non_RT_Chain (ChainId_t id, vector <uint16_t> S_u);
};

// Instruct the compiler to identify (and, in particular, hash) Chains based on theirs id only.
class ChainHash {
	public:
	size_t operator()(const Chain& c) const {
  	return hash<ChainId_t>()(c.id);
  }
};

typedef unordered_set <Chain, ChainHash>                 UnorderedSetOfChains;

/*************************************************************************************************************************************************
Rcvs 2 sorted vectors of chains. 
Put in the first vector (given by ref') a sorted vector, containing the union of the two input vectors. 
**************************************************************************************************************************************************/
void MergeSort (vector <Chain> &vec, const vector <Chain> vec2union);

// Insert a chain in its correct place to a sorted vector of chains
void insertSorted (vector <Chain> &vec, const Chain c); // Insert a chain c to the correct place in the vector, based on its latency tightness.
bool findChainInSet 	 (set<Chain> setOfChains, ChainId_t id, Chain& foundChain); // Given chainId, assigns to chain the respective chain from the set. 
bool eraseChainFromSet (UnorderedSetOfChains &setOfChains, uint16_t chainId); // Given chainId, erases the respective chain from the set. 

/*************************************************************************************************************************************************
Find a chain (given by its id) in a given set of chains.
**************************************************************************************************************************************************/
bool 					findChainInSet  (UnorderedSetOfChains setOfChains , ChainId_t chainId, Chain &c);
vector<Chain> findChainsByPoa (UnorderedSetOfChains setOfChains, uint16_t poa);

/*bool UsesMoreCpu (const Chain& lhs, const Chain& rhs);*/
/*{*/
/*        return lhs.getCpu () >= rhs.getCpu ();*/
/*}*/

/*using UsesMoreCpuType = std::integral_constant<decltype(&UsesMoreCpu), &UsesMoreCpu>;*/

/*set<Chain, UsesMoreCpuType> set1;*/

/*set<Chain, std::integral_constant<decltype(&UsesMoreCpu), &UsesMoreCpu>> set2;*/

#endif


