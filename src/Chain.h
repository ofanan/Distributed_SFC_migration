#pragma once
//#ifndef CHAIN_H
//#define CHAIN_H


#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <omnetpp.h>
#include "Parameters.h"

class Chain
{
public:
    int32_t id;
    int16_t curDatacenter; // Id of the datacenter currently hosting me
    int16_t nxtDatacenter; // Id of the datacenter scheduled to host me
    int16_t curLvl;        // Level of the datacenter currently hosting me // Do we really need this?
    std::vector <int16_t> S_u;         // List of delay-feasible servers
//    int16_t S_u[];         // List of delay-feasible servers
//    bool isNew;        // When true, this chain is new (not currently scheduled to any datacenter). We may get rid of this by setting curDatacenter==-1 to new chains.
    Chain (){};
    Chain (int32_t id, std::vector <int16_t> S_u) {
//        Chain (int32_t id, int16_t S_u[]) {
        this->id = id;
        this->S_u = S_u;
    };
//    explicit Chain (int id);
};

//Chain::Chain ()
//{
//}

//Chain::Chain (int32_t id)
//{
//    this->id = id;
//}

class RT_Chain : public Chain
{
public:
    static const uint8_t mu_u[5]; //{1,5,10};RT_Chain_mu_u;
//    explicit RT_Chain(int x) : Chain(x) {    }
};

//class Non_RT_Chain: public Chain
//{
//public:
//    static const uint8_t mu_u[];
////    explicit Non_RT_Chain(int x) : Chain(x) {    }
//};

typedef std::list<Chain> ChainList;
//typedef Chain[] ChainArray;

//#endif // ifndef CHAIN_H
