#ifndef L2QUEUE_H
#define L2QUEUE_H

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include "Parameters.h"

using namespace omnetpp;

class L2Queue
{
  private:
    cQueue output_queues;
    cMessage *endTransmissionEvent;
    bool isBusy;

    void sndMsg (); // send a message from the local datacenter to the output line. If the line is idle - xmt immediately. Else, Q the msg.

  public:
    L2Queue();
    virtual ~L2Queue();

  protected:
    virtual void handleMessage(cMessage *msg);
    virtual void startTransmitting(cPacket *pkt);

};

//L2Queue::L2Queue()
//{
//    endTransmissionEvent = nullptr;
//    isBusy = false;
////    if (par("useCutThroughSwitching"))
////        gate("line$i")->setDeliverOnReceptionStart(true);
//}

L2Queue::~L2Queue()
{
    cancelAndDelete(endTransmissionEvent);
}

//
//void L2Queue::startTransmitting(cPacket *pkt)
//{
//    EV << "Starting transmission of " << pkt << endl;
//    isBusy = true;
//    int64_t numBytes = check_and_cast<cPacket *>(pkt)->getByteLength(); //
//
//    send(pkt, "line$o");
//
//    // Schedule an event for the time when last bit will leave the gate.
//    simtime_t endTransmissionTime = gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime();
//    scheduleAt(endTransmissionTime, endTransmissionEvent);
//}
//
///*
// * Handle a message. Currently, only self msgs, notifying the end of xmt of the previous pkt, are possible.
// */
//void L2Queue::handleMessage(cMessage *msg)
//{
//    if (msg == endTransmissionEvent) {
//        // Transmission finished, we can start next one.
//        EV << "Transmission finished.\n";
//        isBusy = false;
//        if (!(queue.isEmpty())) {
//            cPacket *pkt;
//            pkt = (cPacket*)queue.pop();
//           startTransmitting(pkt);
//        }
//    }
//    else {
//        EV << "error: L2Q rcvd an unexpected msg\n";
//        endSimulation();
//    }
//}
//
//// send a message from the local datacenter to the output line. If the line is idle - xmt immediately. Else, Q the msg.
//void L2Queue::sndMsg (cMessage *msg) {
//    if (endTransmissionEvent->isScheduled()) {
//        // We Assume infinite Q capacity
//        EV << "Received " << msg << " but transmitter busy: queueing up\n";
//        msg->setTimestamp();
//        queue.insert(msg);
//    }
//    else {
//        // We are idle, so we can start transmitting right away.
//        EV << "xmtng" << msg << endl;
//        startTransmitting((cPacket*)msg);
//    }
//}

#endif // ifndef L2QUEUE_H

