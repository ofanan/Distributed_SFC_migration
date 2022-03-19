#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include "Chain.h"
#include "Parameters.h"

using namespace omnetpp;
const int16_t RT_Chain::S_u[6] = {0,1,2,3,4}; //RT_Chain_S_u
const int8_t  RT_Chain::S_u_size = RT_Chain_S_u_size; //3; //sizeof(RT_Chain::S_u)/sizeof(RT_Chain::S_u[0]);;

const int16_t Non_RT_Chain::S_u[] = {0,1,2,3,4};
const int8_t  Non_RT_Chain::S_u_size = 3; //sizeof(RT_Chain::S_u)/sizeof(RT_Chain::S_u[0]);;

class ChainGenerator : public cSimpleModule
{};

Define_Module(ChainGenerator);

