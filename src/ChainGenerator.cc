#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include "Chain.h"
#include "Parameters.h"

using namespace omnetpp;
const uint8_t RT_Chain::S_u[] = {RT_Chain_S_u[0],RT_Chain_S_u[1],RT_Chain_S_u[2],RT_Chain_S_u[3],RT_Chain_S_u[4]};
const uint8_t  RT_Chain::S_u_size = RT_Chain_S_u_size;

const uint8_t Non_RT_Chain::S_u[] = {Non_RT_Chain_S_u[0],Non_RT_Chain_S_u[1],Non_RT_Chain_S_u[2],Non_RT_Chain_S_u[3],Non_RT_Chain_S_u[4]};
const uint8_t  Non_RT_Chain::S_u_size = Non_RT_Chain_S_u_size;

class ChainGenerator : public cSimpleModule
{};

Define_Module(ChainGenerator);

