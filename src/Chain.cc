#include <stdio.h>
#include <stdint.h>

#include <vector>
#include <set>
#include <algorithm>

#include <omnetpp.h>
#include "Parameters.h"
#include "Chain.h"

void findChainInSet (set<Chain> setOfChains, int32_t req_id, Chain& foundChain) { 
  set<Chain>::iterator it;
  for(it = setOfChains.begin(); it!=setOfChains.end(); ++it){
    if (it -> id == req_id) { // Found the requested chain
      foundChain = *it;    
      return;
    }
  }
}

