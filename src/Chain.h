#pragma once
#include <stdio.h>
#include <stdint.h>

#include <vector>
#include <set>
#include <algorithm>

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
    bool isRtChain;
//    int16_t S_u[];         // List of delay-feasible servers
/*    int16_t curLvl;        // Level of the datacenter currently hosting me // Do we really need this?*/
//    bool isNew;        // When true, this chain is new (not currently scheduled to any datacenter). We may get rid of this by setting curDatacenter==-1 to new chains.
    Chain () {};
    Chain (int32_t id, vector <int16_t> S_u) {
        this->id = id;
        this->S_u = S_u;
        curDatacenter = nxtDatacenter = -1;
    };
        
    // We order chain by non-increasing order of |S_u|, namely how high they can be located in the tree; iteratively breaking ties by decreasing mu_u[l] for each level l, namely,
    // the amount of CPU rsrcs required for locating the chain at a certain lvl.
    // However, in our current sim settings, all of this is degenerated to the fast and efficient rule that an RT chain has higher priority (==smaller #), over a non-RT chain.
    bool operator< (const Chain &right) const {
      return (this->isRtChain && !(right.isRtChain));
    }
};

/*void findChainInSet (set<Chain> setOfChains, int32_t req_id, Chain& foundChain) { */
/*  set<Chain>::iterator it;*/
/*  for(it = setOfChains.begin(); it!=setOfChains.end(); ++it){*/
/*    if (it -> id == req_id) { // Found the requested chain*/
/*      foundChain = *it;    */
/*      return;*/
/*    }*/
/*  }*/
/*}*/

class RT_Chain : public Chain
{
public:
  static const uint8_t mu_u[];
    RT_Chain (int32_t id, vector <int16_t> S_u) {
      this->id        = id;
      this->S_u       = S_u;
      this->isRtChain = true;
      curDatacenter   = nxtDatacenter = -1;
    };
};

class Non_RT_Chain: public Chain
{
  public:
    static const uint8_t mu_u[];
//    explicit Non_RT_Chain(int x) : Chain(x) {    }
    Non_RT_Chain (int32_t id, vector <int16_t> S_u) {
       this->id       = id;
      this->S_u       = S_u;
      this->isRtChain = false;
      curDatacenter   = nxtDatacenter = -1;
    };
};



/*struct findChain {*/
/*    findChain(const int32_t & id) : id(id) {}*/
/*    bool operator()(const Chain & chain) {*/
/*        return chain.id == id;*/
/*    }*/
/*private:*/
/*    int32_t id;*/
/*};*/

//Chain findChainInSet (set<Chain> setOfChains, int32_t id) { 
/*  set<Chain>::iterator iter = find_if (setOfChains.begin(), setOfChains.end(), findChain(id));*/
/*  if (iter==setOfChains.end()) {*/
/*    cout << "error! didn't find the chain\n";*/
/*    exit (0);*/
/*  }*/
/*  else {*/
/*    return *iter;*/
/*  }*/
/*}*/

/*typedef list<Chain> ChainList;*/
//typedef Chain[] ChainArray;

