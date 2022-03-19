#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include "Chain.h"
#include "Parameters.h"

using namespace omnetpp;

/*
Check that the simulations' parameters are valid and feasible.
*/
class ParamChecker : public cSimpleModule
{
private:
        virtual void initialize();
};

Define_Module(ParamChecker);

void ParamChecker::initialize ()
{
	EV << "Hi. I'm ParamChecker. Finishing sim\n";
	endSimulation();

	
}

