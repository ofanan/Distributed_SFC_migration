#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

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

Chain::Chain (int id)
{
    id = id;
}

class SimController : public cSimpleModule
{
private:
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
};

Define_Module(SimController);

void SimController::initialize ()
{
    Chain *chain = new Chain (7);
    EV << "The id of my chain is " << chain->id << "\n";
}

void SimController::handleMessage (cMessage *msg)
{
}
