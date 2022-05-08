
#ifndef CHAIN_H
#define CHAIN_H
#include <stdio.h>
#include <stdint.h>

#include <vector>
#include <set>
#include <algorithm>
#include <unordered_set>

#include <omnetpp.h>
#include "Parameters.h"

using namespace std;


class Chain
{
  public:
    int32_t id;
    int16_t curDatacenter; // Id of the datacenter currently hosting me
    int16_t nxtDatacenter; // Id of the datacenter scheduled to host me
    vector <int16_t> S_u;         // List of delay-feasible servers
    bool isRT_Chain;
/*    int16_t curLvl;        // Level of the datacenter currently hosting me // Do we really need this?*/
//    bool isNew;        // When true, this chain is new (not currently scheduled to any datacenter). We may get rid of this by setting curDatacenter==-1 to new chains.
    Chain () {};
    Chain (int32_t id, vector <int16_t> S_u) {
		this->id = id;
		this->S_u = S_u;
		curDatacenter = nxtDatacenter = -1; 
    };
        
    bool operator== (const Chain &right) const {
      return (this->id == right.id);
    }
		/* 
		We order chain by non-increasing order of |S_u|, namely how high they can be located in the tree; iteratively breaking ties by decreasing mu_u[l] for each level \ell, namely, the amount of CPU 
		rsrcs required for locating the chain at a certain lvl.
		However, in our current sim settings, all of this is degenerated to the fast and efficient rule that an RT chain has higher priority (==smaller #), over a non-RT chain.
		*/
    bool operator< (const Chain &right) const {
      return (this->isRT_Chain && !(right.isRT_Chain));
    }
    
};

/* 
* Accessory function, for finding a chain within a given list of chains.
* We assume that a chain is inequivocally identified by its id.
*/
bool findChainInSet (set<Chain> setOfChains, int32_t id, Chain& foundChain);

class RT_Chain : public Chain
{
public:
  static const vector<uint8_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
  static const uint8_t mu_u_len;
	static const vector<uint16_t> cpuCostAtLvl; // cpuCostAtLvl[i] will hold the cost of placing an RT chain on a DC in level i
  RT_Chain (int32_t id, vector <int16_t> S_u) {
    this->id        = id;
    this->S_u       = S_u;
    this->isRT_Chain = true;
    curDatacenter   = nxtDatacenter = -1;
  };
};

class Non_RT_Chain: public Chain
{
  public:
	  static const vector<uint8_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
	  static const uint8_t  mu_u_len;
		static const vector<uint16_t> cpuCostAtLvl; // cpuCostAtLvl[i] will hold the cost of placing a non- RT chain on a DC in level i
//    explicit Non_RT_Chain(int x) : Chain(x) {    }
    Non_RT_Chain (int32_t id, vector <int16_t> S_u) {
       this->id       = id;
      this->S_u       = S_u;
      this->isRT_Chain = false;
      curDatacenter   = nxtDatacenter = -1;
    };
};

// Instruct the compiler to identify (and, in particular, hash) Chains based on theirs id only.
struct ChainHash {
	size_t operator()(const Chain& c) const {
  	return hash<int>()(c.id);
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
bool findChainInSet (unordered_set <Chain, ChainHash> setOfChains , int32_t chainId, Chain &c);

#endif

/*typedef list<Chain> ChainList;*/
//typedef Chain[] ChainArray;

