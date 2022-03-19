#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include "Parameters.h"

using namespace omnetpp;

class Datacenter : public cSimpleModule
{
public:
    int16_t  availCpu;
    std::vector <int32_t> assigned_chains;
    std::vector <int32_t> placed_chains;

private:
        virtual void initialize();
        virtual void handleMessage (cMessage *msg);
 };

//
Define_Module(Datacenter);

void Datacenter::initialize()
{
    int16_t id      = (int)(par("id"));
    availCpu        = nonAugmentedCpuAtLvl[int(par("lvl"))]; // Consider rsrc aug here?
    assigned_chains = {};
    placed_chains   = {};
//    nonAugmentedCpu =
//    if (id==1) {
//        EV << "my node id is " << id << endl;
//        cPacket *pkt = new cPacket("dummy");
//        pkt->setBitLength (10000);
//        send(pkt, "toChild$o", 0);
//        pkt = new cPacket("dummy");
//        pkt->setBitLength (10000);
//        send(pkt, "toChild$o", 0);
//    }

}

void Datacenter::handleMessage (cMessage *msg)
{
    EV << "DC " << int (par ("id")) << " rcvd msg";
}


class L2Queue : public cSimpleModule
{
  private:
    long frameCapacity;

    cQueue queue;
    cMessage *endTransmissionEvent;
    bool isBusy;

  public:
    L2Queue();
    virtual ~L2Queue();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void startTransmitting(cPacket *pkt);
};

//Define_Module(L2Queue);
//
//L2Queue::L2Queue()
//{
//    endTransmissionEvent = nullptr;
//}
//
//L2Queue::~L2Queue()
//{
//    cancelAndDelete(endTransmissionEvent);
//}
//
//void L2Queue::initialize()
//{
//    queue.setName("queue");
//    endTransmissionEvent = new cMessage("endTxEvent");
//
//    if (par("useCutThroughSwitching"))
//        gate("line$i")->setDeliverOnReceptionStart(true);
//
//    frameCapacity = par("frameCapacity");
//
//    isBusy = false;
//}
//
//void L2Queue::startTransmitting(cPacket *pkt)
//{
//    EV << "Starting transmission of " << pkt << endl;
//    isBusy = true;
//    int64_t numBytes = check_and_cast<cPacket *>(pkt)->getByteLength(); //#$$$
//
//    send(pkt, "line$o");
//
//    // Schedule an event for the time when last bit will leave the gate.
//    simtime_t endTransmissionTime = gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime();
//    scheduleAt(endTransmissionTime, endTransmissionEvent);
//}
//
//void L2Queue::handleMessage(cMessage *msg)
//{
//    if (msg == endTransmissionEvent) {
//        // Transmission finished, we can start next one.
//        EV << "Transmission finished.\n";
//        isBusy = false;
//        if (!(queue.isEmpty())) {
//            cPacket *pkt;
//            pkt = (cPacket  *)queue.pop();
//           startTransmitting(pkt);
//        }
//    }
//    else if (msg->arrivedOn("line$i")) {
//        // pass msg up to the local datacenter
//        send(msg, "local$o");
//    }
//    else {  // arrived on local port, connected to the local datacenter (and should be sent out, on the line)
//        if (endTransmissionEvent->isScheduled()) {
//            // We Assume infinite Q capacity
//            EV << "Received " << msg << " but transmitter busy: queueing up\n";
//            msg->setTimestamp();
//            queue.insert(msg);
//        }
//        else {
//            // We are idle, so we can start transmitting right away.
//            EV << "Received " << msg << endl;
//            startTransmitting((cPacket*)msg);
//        }
//    }
//}
//
//void L2Queue::refreshDisplay() const
//{
//    getDisplayString().setTagArg("t", 0, isBusy ? "transmitting" : "idle");
//    getDisplayString().setTagArg("i", 1, isBusy ? (queue.getLength() >= 3 ? "red" : "yellow") : "");
//}
//simple L2Queue
//{
//    parameters:
//        int frameCapacity = default(0); // max number of packets; 0 means no limit
//        bool useCutThroughSwitching = default(false);  // use cut-through switching instead of store-and-forward
//        @display("i=block/queue;q=queue");
//
//    gates:
//        inout local; // to/from the local datacenter
//        inout line; // the line to/from other datacenter
//}
//
//
