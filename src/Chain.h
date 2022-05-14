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

class Chain
{
  public:
    uint32_t id;
    vector <uint16_t> S_u;         // List of delay-feasible datacenters for this chain
    bool isRT_Chain;
		const static vector<uint16_t> costOfCpuUnitAtLvl; 
		int8_t curLvl;        // Level of the datacenter currently hosting me 
		//    bool isNew;        // When true, this chain is new (not currently scheduled to any datacenter). We may get rid of this by setting curDatacenter==-1 to new chains.
    Chain () {};

    Chain (uint32_t id, vector <uint16_t> S_u);
            
    bool operator== (const Chain &right) const {
      return (this->id == right.id);
    }

		// Getters
		int16_t 	getCurDatacenter () const; // returns the id of the datacenter currently hosting this; or UNPLACED, if this chain isn't placed
    uint16_t 	getCpuCost () const;
    uint16_t 	getCpu     () const;
    
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
};

/* 
* Accessory function, for finding a chain within a given list of chains.
* We assume that a chain is inequivocally identified by its id.
*/
bool findChainInSet (set<Chain> setOfChains, uint32_t id, Chain& foundChain);

class RT_Chain : public Chain
{
public:
  static const vector<uint16_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
  static const uint8_t mu_u_len;
	static const vector<uint16_t> cpuCostAtLvl; // cpuCostAtLvl[i] will hold the cost of placing an RT chain on a DC in level i
  RT_Chain (uint32_t id, vector <uint16_t> S_u);
};

class Non_RT_Chain: public Chain
{
  public:
	  static const vector<uint16_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
	  static const uint8_t  mu_u_len;
		static const vector<uint16_t> cpuCostAtLvl; // cpuCostAtLvl[i] will hold the cost of placing a non- RT chain on a DC in level i
    Non_RT_Chain (uint32_t id, vector <uint16_t> S_u);
};

// Instruct the compiler to identify (and, in particular, hash) Chains based on theirs id only.
struct ChainHash {
	size_t operator()(const Chain& c) const {
  	return hash<uint32_t>()(c.id);
  }
};

/*
Find a chain (given by its id) in a given set of chains.
Inputs:
- The set of chains.
- Id of the requested chain.
- Ref' to which the found chain is written.
Output: true iff the requested chain is found.
*/
bool findChainInSet (unordered_set <Chain, ChainHash> setOfChains , uint32_t chainId, Chain &c);

/*
Insert a chain in its correct place to a sorted vector of chains
*/
void insertSorted (vector <Chain> &vec, const Chain c);

/*
Rcvs 2 sorted vectors of chains. 
Put in the first vector (given by ref') a sorted vector, containing the union of the two input vectors. 
*/
void MergeSort (vector <Chain> &vec, const vector <Chain> vec2union);

typedef unordered_set <Chain, ChainHash> SetOfChains;

/*void modifyCurLvl (SetOfChains &setOfChins, const Chain chain, const int8_t curLvl);*/
/*void SetOfChains::modifyCurLvl (const Chain chain, const int8_t curLvl);*/
#endif


