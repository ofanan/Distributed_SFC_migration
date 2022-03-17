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
//#include "Node.h";

using namespace omnetpp;

class Datacenter : public cSimpleModule
{
    private:
        virtual void initialize();
        virtual void handleMessage (cMessage *msg);
        cSimpleModule* prnt;
        bool isRoot;
        bool isLeaf;
        int nodeId;
};

//
Define_Module(Datacenter);

void Datacenter::initialize()
{
    prnt        = (cSimpleModule*) (getParentModule());
    isRoot      = prnt->par ("isRoot");
    isLeaf      = ((int)(par ("numChildren"))==0);

    nodeId      = (int) (prnt->par("nodeId"));
    if (nodeId==1) {
        EV << "my node id is " << nodeId << endl;
        cPacket *pkt = new cPacket("dummy");
        pkt->setBitLength (10000);
        send(pkt, "toChild$o", 0);
        pkt = new cPacket("dummy");
        pkt->setBitLength (10000);
        send(pkt, "toChild$o", 0);
        pkt = new cPacket("dummy");
        pkt->setBitLength (10000);
        send(pkt, "toChild$o", 0);
    }

}

void Datacenter::handleMessage (cMessage *msg)
{
    EV << "node " << nodeId << " rcvd msg";

}


