//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003 Ahmet Sekercioglu
// Copyright (C) 2003-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Datacenter : public cSimpleModule
{
    private:
        virtual void initialize();
        virtual void handleMessage (cMessage *msg);
};

//
Define_Module(Datacenter);

void Datacenter::initialize()
{
//    int numDatacenters = par ("numDatacenters");
//    if (getIndex()==numDatacenters-1) {
//        EV << "node " << getIndex () << " is sending msg";
//        cMessage *msg = new cMessage("dummy");
//        send(msg, "toParent$o");
//    }

}

void Datacenter::handleMessage (cMessage *msg)
{
    EV << "node " << getIndex() << " rcvd msg";
    if (getIndex() == 0) {
        EV << " sending msg to child 2";
        cMessage *msg = new cMessage("dummy");
        send(msg, "toChild$o", 1);
    }
}


