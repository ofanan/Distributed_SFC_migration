#ifndef PARAMS_H
#define PARAMS_H

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "Chain.h"

const int HEIGHT=5;
//const uint8_t RT_Chain_mu_u[]     = {1,5,10};
//const uint8_t RT_Chain::mu_u[]  = {1, 5, 10}; //{RT_Chain_mu_u[0], RT_Chain_mu_u[1], RT_Chain_mu_u[2]};
//const uint8_t Non_RT_Chain_mu_u[] = {1,2,3,4};
//const uint8_t RT_Chain::mu_u[]      = {RT_Chain_mu_u[0],RT_Chain_mu_u[1],RT_Chain_mu_u[2],RT_Chain_mu_u[3],RT_Chain_mu_u[4]};

const int16_t nonAugmentedCpuAtLvl[HEIGHT] = {10,20,30,40,50};

const int16_t chainMigCost = 600;

const int16_t costOfPlacingChainAtLvl[HEIGHT] = {10,20,30,40,50};

#endif // ifndef PARAMS_H
