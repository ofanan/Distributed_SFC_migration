#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
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

Define_Module(L2Queue);

L2Queue::L2Queue()
{
    endTransmissionEvent = nullptr;
}

L2Queue::~L2Queue()
{
    cancelAndDelete(endTransmissionEvent);
}

void L2Queue::initialize()
{
    queue.setName("queue");
    endTransmissionEvent = new cMessage("endTxEvent");

    if (par("useCutThroughSwitching"))
        gate("line$i")->setDeliverOnReceptionStart(true);

    frameCapacity = par("frameCapacity");

    isBusy = false;
}

void L2Queue::startTransmitting(cPacket *pkt)
{
    EV << "Starting transmission of " << pkt << endl;
    isBusy = true;
    int64_t numBytes = check_and_cast<cPacket *>(pkt)->getByteLength(); //#$$$

    send(pkt, "line$o");

    // Schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmissionTime = gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime();
    scheduleAt(endTransmissionTime, endTransmissionEvent);
}

void L2Queue::handleMessage(cMessage *msg)
{
    if (msg == endTransmissionEvent) {
        // Transmission finished, we can start next one.
        EV << "Transmission finished.\n";
        isBusy = false;
        if (!(queue.isEmpty())) {
            cPacket *pkt;
            pkt = (cPacket  *)queue.pop();
           startTransmitting(pkt);
        }
    }
    else if (msg->arrivedOn("line$i")) {
        // pass up
        send(msg, "out");
    }
    else {  // arrived on local port, connected to the local datacenter (and should be sent out, on the line)
        if (endTransmissionEvent->isScheduled()) {
            // We Assume infinite Q capacity, and therefore comment the lines below
//            // We are currently busy, so just queue up the packet.
//            if (frameCapacity && queue.getLength() >= frameCapacity) {
//                EV << "Received " << pkt << " but transmitter busy and queue full: discarding\n";
//                delete pkt;
//            }
//            else {
                EV << "Received " << msg << " but transmitter busy: queueing up\n";
                msg->setTimestamp();
                queue.insert(msg);
//            }
        }
        else {
            // We are idle, so we can start transmitting right away.
            EV << "Received " << msg << endl;
            startTransmitting((cPacket*)msg);
        }
    }
}

void L2Queue::refreshDisplay() const
{
    getDisplayString().setTagArg("t", 0, isBusy ? "transmitting" : "idle");
    getDisplayString().setTagArg("i", 1, isBusy ? (queue.getLength() >= 3 ? "red" : "yellow") : "");
}

//cChannel *channel = gate->getChannel(); // 4.6.3
//getTransmissionChannel()
//cPacket *pkt = ...; // packet to be transmitted
//cChannel *txChannel = gate("out")->getTransmissionChannel();
//simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
//if (txFinishTime <= simTime())
//{
//    // channel free; send out packet immediately
//    send(pkt, "out");
//}
//else
//{
//    // store packet and schedule timer; when the timer expires,
//    // the packet should be removed from the queue and sent out
//    txQueue.insert(pkt);
//    scheduleAt(txFinishTime, endTxMsg);
//}
