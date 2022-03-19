#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include "Chain.h"
#include "Parameters.h"

using namespace omnetpp;
const int16_t RT_Chain::S_u[] = {RT_Chain_S_u[0],RT_Chain_S_u[1],RT_Chain_S_u[2],RT_Chain_S_u[3],RT_Chain_S_u[4]};
const int8_t  RT_Chain::S_u_size = RT_Chain_S_u_size; //3; //sizeof(RT_Chain::S_u)/sizeof(RT_Chain::S_u[0]);;

const int16_t Non_RT_Chain::S_u[] = {0,1,2,3,4};
const int8_t  Non_RT_Chain::S_u_size = 3; //sizeof(RT_Chain::S_u)/sizeof(RT_Chain::S_u[0]);;

class ChainGenerator : public cSimpleModule
{};

Define_Module(ChainGenerator);

