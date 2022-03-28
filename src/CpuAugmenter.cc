#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
//#include "Chain.h"
#include "Parameters.h"

using namespace omnetpp;

class CpuAugmenter : public cSimpleModule
{
public:
    double rsrcAug = 1.0;
};

Define_Module(CpuAugmenter);

