#pragma once
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "Chain.h"

const int HEIGHT=5;

const int16_t nonAugmentedCpuAtLvl[HEIGHT] = {10,20,30,40,50};

const int16_t chainMigCost = 600;

const int16_t costOfPlacingChainAtLvl[HEIGHT] = {10,20,30,40,50};

/*void findChainInSet (set<Chain> setOfChains, int32_t req_id, Chain& foundChain) { */
/*  set<Chain>::iterator it;*/
/*  for(it = setOfChains.begin(); it!=setOfChains.end(); ++it){*/
/*    if (it -> id == req_id) { // Found the requested chain*/
/*      foundChain = *it;    */
/*      return;*/
/*    }*/
/*  }*/
/*}*/

