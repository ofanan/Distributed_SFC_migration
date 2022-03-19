#ifndef CHAIN_H
#define CHAIN_H

#include <stdio.h>
#include <stdint.h>
#include <vector>

class Chain
{
public:
    int id;
    int16_t curDatacenter; // Id of the datacenter currently hosting me
    int16_t nxtDatacenter; // Id of the datacenter scheduled to host me
    int8_t curLvl;        // Level of the datacenter currently hosting me
//    bool isRtChain;    // When true, this is a RT chain
    bool isNew;        // When true, this chain is new (not currently scheduled to any datacenter)
    explicit Chain (int id) {this->id = id;};
};

class RT_Chain: public Chain
{
public:
    static const int S_u[5];

//    static const int8_t S_u[2];
//    static const int8_t S_u = 5; // Works!
//    enum {S_u_enum = 2};
//    int8_t S_uu[] = {1, 2};
//    static std::vector <int8_t> S_u; // = [1, 2];
    explicit RT_Chain(int x) : Chain(x) {
    }
};


#endif // ifndef CHAIN_H

//Code a "chain generator", connected to the TraceFeeder. It
