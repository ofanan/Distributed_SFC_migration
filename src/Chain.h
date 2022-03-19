#ifndef CHAIN_H
#define CHAIN_H

#include <stdio.h>
#include <stdint.h>

//class Chain
//{
//public:
//    int my_x;
//    explicit Chain(int x) {
//        my_x = x;
//    }
//};

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

//class RT_Chain: public Chain
//{
//public:
//    int my_y;
//    explicit RT_Chain(int x) : Chain(x) {
//        my_y = 7;
//    }
//};

class RT_Chain: public Chain
{
public:
    int my_y;
    explicit RT_Chain(int x) : Chain(x) {
        my_y = 7;
    }
};


#endif // ifndef CHAIN_H

//Code a "chain generator", connected to the TraceFeeder. It
