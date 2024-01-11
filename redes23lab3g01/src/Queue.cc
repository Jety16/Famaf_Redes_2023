#ifndef QUEUE
#define QUEUE

#include <omnetpp.h>
#include <string.h>

using namespace omnetpp;


class TransportPacket : public cPacket {
    bool slowDown;
    bool speedUp;
    int bufferSize;

    public:
        TransportPacket(const char *name = "TransportPacket", short kind = 2) : cPacket(name, kind) {}

        int getBufferSize() const { return this->bufferSize; }
        void setBufferSize(int bufferSize) {this->bufferSize = bufferSize; }

        bool getSlowDown() const { return this->slowDown; }
        void setSlowDown(bool b) { this->slowDown = b; }

        bool getSpeedUp() const { return this->speedUp; }
        void setSpeedUp(bool b) { this->speedUp = b; }
};


class Queue: public cSimpleModule {
private:
    cQueue buffer;
    cOutVector packetDropVector;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    bool isSlowed;
public:
    Queue();
    cOutVector bufferSizeVector;
    virtual ~Queue();

   protected:
    virtual void initialize();
    virtual cQueue getBuffer() { return this->buffer; };
    virtual simtime_t getServiceTime() { return this->serviceTime; };
    virtual cMessage *getEndServiceEvent() { return endServiceEvent; }
    virtual void setServiceTime(simtime_t serviceTime) { this->serviceTime = serviceTime; };
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Queue);

Queue::Queue() {
    endServiceEvent = NULL;
}

Queue::~Queue() {
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize() {
    buffer.setName("buffer");
    packetDropVector.setName("packetDropVector");
    bufferSizeVector.setName("bufferSize");
    endServiceEvent = new cMessage("endService");
    isSlowed = false;
}

void Queue::finish() {
}

void Queue::handleMessage(cMessage *msg) {
    // this->bubble("Hola soy queue");
    //  if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket *)buffer.pop();
            // send packet
            send(pkt, "out");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else {  // if msg is a data packet
        // enqueue the packet

        // check buffer limit
        // cambiado a intValue pq sino no me tira error
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // drop the packet
            delete msg;
            this->bubble("packet dropped");
            packetDropVector.record(1);
        }
        else{
            if (not isSlowed and (buffer.getLength() > par("bufferSize").intValue() * 0.49)) {
                isSlowed = true;
                TransportPacket *pkt = new TransportPacket();
                pkt->setByteLength(20);
                pkt->setBufferSize(par("bufferSize").intValue() - buffer.getLength());
                pkt->setSlowDown(false);
                pkt->setSpeedUp(false);
                pkt->setSlowDown(true);
                scheduleAt(simTime() + 0, pkt);
            }
            if (isSlowed and (buffer.getLength() < par("bufferSize").intValue() * 0.4)){
                isSlowed = false;
                TransportPacket *pkt = new TransportPacket();
                pkt->setByteLength(20);
                pkt->setBufferSize(par("bufferSize").intValue() - buffer.getLength());
                pkt->setSlowDown(false);
                pkt->setSpeedUp(true);
                pkt->setSlowDown(false);
                scheduleAt(simTime() + 0, pkt);
            }
            buffer.insert(msg);
            bufferSizeVector.record(buffer.getLength());
            // if the server is idle
            if (!endServiceEvent->isScheduled()) {
                // start the service
                scheduleAt(simTime() + 0, endServiceEvent);
            }

        }
    }
};


class TransportRx: public cSimpleModule {
private:
    bool isSlowed;
    cQueue buffer;
    cOutVector packetDropVector;
    cOutVector bufferSizeVector;
    cMessage *endServiceEvent;
    cMessage *feedBackServiceEvent;
    simtime_t serviceTime;

   public:
    TransportRx();
    virtual ~TransportRx();

   protected:
    virtual void initialize();
    virtual cQueue getBuffer() { return this->buffer; };
    virtual simtime_t getServiceTime() { return this->serviceTime; };
    virtual cMessage *getEndServiceEvent() { return endServiceEvent; }
    virtual void setServiceTime(simtime_t serviceTime) { this->serviceTime = serviceTime; };
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportRx);

TransportRx::TransportRx() {
    endServiceEvent = NULL;
}

TransportRx::~TransportRx() {
    cancelAndDelete(endServiceEvent);
}

void TransportRx::initialize() {
    buffer.setName("bufferRx");

    packetDropVector.setName("packetDropVector");
    bufferSizeVector.setName("bufferSize");
    endServiceEvent = new cMessage("endService");
    isSlowed = false;
}

void TransportRx::finish() {
}

void TransportRx::handleMessage(cMessage *msg) {
    if (msg->getKind() == 2){
            // msg is feedback
            TransportPacket* pkt = (TransportPacket*)msg;
            send(pkt, "toOut$o");
    } else if (msg->getKind() == 0) {
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket *)buffer.pop();
            // send packet
            send(pkt, "toApp");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else {  // if msg is a data packet
        // enqueue the packet

        // check buffer limit
        // cambiado a intValue pq sino no me tira error
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // drop the packet
            delete msg;
            this->bubble("packet dropped");
            packetDropVector.record(1);
        } else {
            buffer.insert(msg);
            bufferSizeVector.record(buffer.getLength());
            // if the server is idle
            if (!endServiceEvent->isScheduled()) {
                // start the service
                scheduleAt(simTime(), endServiceEvent);
            }

            if (not isSlowed and (buffer.getLength() > par("bufferSize").intValue() * 0.8)) {
                isSlowed = true;
                TransportPacket *pkt = new TransportPacket();
                pkt->setByteLength(20);
                pkt->setBufferSize(par("bufferSize").intValue() - buffer.getLength());
                pkt->setSlowDown(false);
                pkt->setSpeedUp(false);
                pkt->setSlowDown(true);
                send(pkt, "toOut$o");
            }
            if (isSlowed and (buffer.getLength() < par("bufferSize").intValue() * 0.6)) {
                isSlowed = false;
                TransportPacket *pkt = new TransportPacket();
                pkt->setByteLength(20);
                pkt->setBufferSize(par("bufferSize").intValue() - buffer.getLength());
                pkt->setSlowDown(false);
                pkt->setSpeedUp(true);
                pkt->setSlowDown(false);
                send(pkt, "toOut$o");
            }
        }
    }
    }
    };

class TransportTx : public cSimpleModule {
   private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;

    float simTimeOffset;

   public:
    TransportTx();
    cOutVector bufferSizeVector;
    cOutVector packetSend;
    virtual ~TransportTx();

   protected:
    virtual void initialize();
    virtual cQueue getBuffer() { return this->buffer; };
    virtual simtime_t getServiceTime() { return this->serviceTime; };
    virtual cMessage *getEndServiceEvent() { return endServiceEvent; }
    virtual void setServiceTime(simtime_t serviceTime) { this->serviceTime = serviceTime; };
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportTx);

TransportTx::TransportTx() {
    endServiceEvent = NULL;
}

TransportTx::~TransportTx() {
    cancelAndDelete(endServiceEvent);
}

void TransportTx::initialize() {
    buffer.setName("bufferTx");
    packetSend.setName("packetSend");
    bufferSizeVector.setName("bufferSize");
    simTimeOffset = 1;
    endServiceEvent = new cMessage("endService");
}

void TransportTx::finish() {
}

void TransportTx::handleMessage(cMessage *msg) {
    if (msg->getKind() == 2) {
        // msg is feedback
        TransportPacket *pkt = (TransportPacket *)msg;
        if (pkt->getSlowDown()) {
            // slow down
            simTimeOffset = simTimeOffset * 2.5;
        } else if (pkt->getSpeedUp()) {
            // speed up
            simTimeOffset = 1;
        }
    } else if (msg->getKind() == 0) {
        // if msg is signaling an endServiceEvent
        if (msg == endServiceEvent) {
            // if packet in buffer, send next one
            if (!buffer.isEmpty()) {
                // dequeue packet
                cPacket *pkt = (cPacket *)buffer.pop();
                // send packet
                send(pkt, "toOut$o");
                // start new service
                serviceTime = pkt->getDuration() * simTimeOffset;
                scheduleAt(simTime() + serviceTime, endServiceEvent);
                packetSend.record(1);
            }
        } else {  // if msg is a data packet
            // enqueue the packet

            // check buffer limit
            // cambiado a intValue pq sino no me tira error
            if (buffer.getLength() >= par("bufferSize").intValue()) {
                // drop the packet
                delete msg;
                this->bubble("packet dropped");
            } else {
                buffer.insert(msg);
                bufferSizeVector.record(buffer.getLength());
                // if the server is idle
                if (!endServiceEvent->isScheduled()) {
                    // start the service
                    scheduleAt(simTime() * simTimeOffset, endServiceEvent);
                }
            }
        }
    }
};

#endif /* QUEUE */
