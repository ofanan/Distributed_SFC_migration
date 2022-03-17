#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class SimController : public cSimpleModule
{
private:
    virtual void initialize();
    virtual void handleMessage (cMessage *msg);
};

Define_Module(SimController);

void SimController::initialize ()
{
}

void SimController::handleMessage (cMessage *msg)
{
}
