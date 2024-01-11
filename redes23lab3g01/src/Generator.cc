#ifndef GENERATOR
#define GENERATOR

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Generator : public cSimpleModule {
private:
    cMessage *sendMsgEvent;
    cStdDev transmissionStats;
public:
    Generator();
    virtual ~Generator();
    cOutVector packetGenVector;
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};
Define_Module(Generator);

Generator::Generator() {
    sendMsgEvent = NULL;
}

Generator::~Generator() {
    cancelAndDelete(sendMsgEvent);
}
//4.4.1.7 Pros and Cons of Using handleMessage()
//Pros:

//consumes less memory: no separate stack needed for simple modules
//fast: function call is faster than switching between coroutines
//Cons:

//local variables cannot be used to store state information
//need to redefine initialize()

//Modules with handleMessage() are NOT started automatically:
//the simulation kernel creates starter messages only for modules
//with activity(). This means that you have to schedule self-messages
//from the initialize() function if you want a handleMessage() simple
//module to start working “by itself”, without first receiving a message
//from other modules.
//Redefine handleMessage() or specify non-zero stack size to use activity()
//-- in module (Generator) Network.nodeTx.gen (id=7), at t=0.079587450457s,
//event #1
void Generator::initialize() {
    transmissionStats.setName("TotalTransmissions");
    // create the send packet
    sendMsgEvent = new cMessage("sendEvent");
    // me pide que inicialize el size esto es lo unico que se me ocurrio
    //sendMsgEvent->setByteLength(12500);
    // schedule the first event at random time
    //linea de chat para solucion :scheduleAt(simTime(), sendMsgEvent);
    packetGenVector.setName("packetGen");
    scheduleAt(par("generationInterval"),sendMsgEvent);
}

void Generator::finish() {
}

void Generator::handleMessage(cMessage *msg) {

    // create new packet
    // create new packet
    cPacket *pkt = new cPacket("packet");
    pkt->setByteLength(par("packetByteSize"));
    // send to the output
    send(pkt, "out");
    packetGenVector.record(1);
    // compute the new departure time
    simtime_t departureTime = simTime() + par("generationInterval");
    // schedule the new packet generation
    scheduleAt(departureTime, sendMsgEvent);
}

#endif /* GENERATOR */
