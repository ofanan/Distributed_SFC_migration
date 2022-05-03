#include <stdio.h>
#include <stdint.h>

#include <vector>
#include <set>
#include <algorithm>

#include <omnetpp.h>
#include "Parameters.h"
#include "Chain.h"

const uint8_t RT_Chain::mu_u[]      = {1, 5, 10};
const uint8_t Non_RT_Chain::mu_u[]  = {1,2,3,4};

const uint8_t RT_Chain	  ::mu_u_len = 3;
const uint8_t Non_RT_Chain::mu_u_len = 4;

/* 
ind a chain within a given set of chains.
We assume that a chain is inequivocally identified by its id.
Inputs: 
- setOfChains: look for the chain within this set.
- id: of the requested chain.
- &foundChain: a reference, to which the found chain (if there exists) will be written.
Return value: true iff the requested chain was found.
*/
bool findChainInSet (set<Chain> setOfChains, int32_t id, Chain& foundChain) { 
	for (auto const& chain : setOfChains) {
    if (chain.id == id) { // Found the requested chain
      foundChain = chain;    
      return true;
    }
  }
  return false;

//  set<Chain>::iterator it;
//  for(it = setOfChains.begin(); it!=setOfChains.end(); ++it){
//    if (it -> id == id) { // Found the requested chain
//      foundChain = *it;    
//      return true;
//    }
//  }
//  return false;
}

