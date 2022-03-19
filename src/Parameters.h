#ifndef PARAMS_H
#define PARAMS_H

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "Chain.h"

const int MAX_HEIGHT=5;
const uint8_t RT_Chain_S_u[] = {0,1,2,3,4};
const uint8_t RT_Chain_S_u_size = 3;

const uint8_t Non_RT_Chain_S_u[] = {0,1,2,3,4};
const uint8_t Non_RT_Chain_S_u_size = 2;

const int16_t nonAugmentedCpuAtLvl[] = {10,20,30,40,50,60};

const int16_t chainMigCost = 600;

const int16_t costOfPlacingChainInLvl[] = {10,20,30,40,50,60};

#endif // ifndef PARAMS_H
