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
* Find a chain within a given set of chains.
* We assume that a chain is inequivocally identified by its id.
*/
void findChainInSet (set<Chain> setOfChains, int32_t req_id, Chain& foundChain) { 
  set<Chain>::iterator it;
  for(it = setOfChains.begin(); it!=setOfChains.end(); ++it){
    if (it -> id == req_id) { // Found the requested chain
      foundChain = *it;    
      return;
    }
  }
}

