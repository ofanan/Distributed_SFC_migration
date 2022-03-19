#ifndef CHAIN_H
#define CHAIN_H

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "Parameters.h"

class Chain
{
public:
    int32_t id;
    int16_t curDatacenter; // Id of the datacenter currently hosting me
    int16_t nxtDatacenter; // Id of the datacenter scheduled to host me
    int8_t curLvl;        // Level of the datacenter currently hosting me
//    bool isRtChain;    // When true, this is a RT chain
    bool isNew;        // When true, this chain is new (not currently scheduled to any datacenter)
    explicit Chain (int id) {this->id = id;};
};

class RT_Chain : public Chain
{
public:
    static const uint8_t S_u[];
    static const uint8_t S_u_size;
    explicit RT_Chain(int x) : Chain(x) {
    }
};

class Non_RT_Chain: public Chain
{
public:
    static const uint8_t S_u[];
    static const uint8_t S_u_size;
    explicit Non_RT_Chain(int x) : Chain(x) {
    }
};

#endif // ifndef CHAIN_H

//Code a "chain generator", connected to the TraceFeeder. It
