#pragma once
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "Chain.h"
#include "MyConfig.h"

const int HEIGHT=5;

const int16_t nonAugmentedCpuAtLvl[HEIGHT] = {10,20,30,40,50};

const int16_t chainMigCost = 600;

//const int16_t costOfPlacingChainAtLvl[HEIGHT] = {10,20,30,40,50};

//const int8_t  RT_ChainCpuCostAtLvl = MyConfig::dotProdcut (RT_Chain::mu_u, cpuCostAtLvl);

const int16_t UNPLACED = -1;

const bool SYNC  = true;
const bool ASYNC = false;

const int16_t cpuCostAtLvl[HEIGHT] = {5,4,3,2,1}; // 


