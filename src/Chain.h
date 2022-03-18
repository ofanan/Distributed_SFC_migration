#ifndef CHAIN_H
#define CHAIN_H

class Chain
{
public:
    int id;
    int curDatacenter; // Id of the datacenter currently hosting me
    int nxtDatacenter; // Id of the datacenter scheduled to host me
    int curLvl;        // Level of the datacenter currently hosting me
    bool isRtChain;    // When true, this is a RT chain
    bool isNew;        // When true, this chain is new (not currently scheduled to any datacenter)
    Chain (int id);
};


#endif // ifndef CHAIN_H
