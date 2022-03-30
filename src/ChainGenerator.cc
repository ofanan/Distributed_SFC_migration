#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
//#include "Chain.h"
#include "Parameters.h"

using namespace omnetpp;

const uint8_t RT_Chain::mu_u[]  = {1, 5, 10}; //{RT_Chain_mu_u[0], RT_Chain_mu_u[1], RT_Chain_mu_u[2]};
//const uint8_t Non_RT_Chain::mu_u[]  = {Non_RT_Chain_mu_u[0],Non_RT_Chain_mu_u[1],Non_RT_Chain_mu_u[2],Non_RT_Chain_mu_u[3],Non_RT_Chain_mu_u[4]};

class ChainGenerator : public cSimpleModule
{};

Define_Module(ChainGenerator);

