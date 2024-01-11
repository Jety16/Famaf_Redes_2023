#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

class TransportPacket : public cPacket
{
    int destination;
    int length;
public:
    struct idList
       {
           int id;
           int hops;
           idList *next;
       };
    idList *list;
    TransportPacket(const char *name = "TransportPacket", short kind = -1) : cPacket(name, kind) {
        list = NULL;
    }
    void setDestination(int id){
        destination = id;
    }
    int getDestination(){
        return destination;
    }
    void addIdList(int id)
    {
        // add id list to the end of the current list
        idList *aux = this->list;
        if (aux == NULL)
        {
            this->list = new idList;
            this->list->id = id;
            this->list->hops = 0;
            this->list->next = NULL;
            this->length = 1;
        }
        else
        {
            while (aux->next != NULL)
            {
                aux = aux->next;
            }
            aux->next = new idList;
            aux->next->id = id;
            aux->next->hops = aux->hops + 1;
            aux->next->next = NULL;
            this->length += 1;
        }
    }
    int getLength(){
        return length;
    }
    idList *getIdList()
    {
        return list;
    }
};


class Net : public cSimpleModule
{
private:
    cOutVector hopsVector;
    //  cStdDev hopsStats;
    cOutVector LnkSelectVector;
    int* nodeIndexArray;  // Pointer to the array
    int netLength;

public:
    Net();
    virtual ~Net();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net()
{
}

Net::~Net()
{
}

void Net::initialize()
{
    hopsVector.setName("Hops");
    LnkSelectVector.setName("Lnk_Selected");

    TransportPacket *pkt = new TransportPacket();
    pkt->setDestination(this->getParentModule()->getIndex());
    pkt->addIdList(this->getParentModule()->getIndex());
    netLength = 0;
    send(pkt, "toLnk$o", 0);
    //  hopsStats.setName("TotalHops");
}


void Net::finish()
{
    delete[] nodeIndexArray;  // Deallocate memory for the array
    //    recordScalar("AverageHops", hopsStats.getMean());
}

void Net::handleMessage(cMessage *msg)
{

    if (msg->getKind() == -1)
    {
        TransportPacket *pkt = (TransportPacket *)msg;

        this->bubble("recibi tipo 4");
        if (pkt->getDestination() == this->getParentModule()->getIndex())
        {
            this->netLength = pkt->getLength();
            this->nodeIndexArray = new int[this->netLength];

            TransportPacket::idList *aux = pkt->getIdList();
            while (aux != NULL)
            {
                this->nodeIndexArray[aux->hops] = aux->id;
                aux = aux->next;
            }
        }
        else
        {
            pkt->addIdList(this->getParentModule()->getIndex());
            send(msg, "toLnk$o", 0);
        }
    }
    else
    {

        // All msg (events) on net are packets
        Packet *pkt = (Packet *)msg;

        // If this node is the final destination, send to App

        if (pkt->getDestination() == this->getParentModule()->getIndex())
        {

            hopsVector.record(pkt->getHopCount());
            // hopsStats.collect(pkt->getHopCount());
            send(msg, "toApp$o");
        }
        // If not, forward the packet to some else... to who?
        else
        {
            // We send to link interface #0, which is the
            // one connected to the clockwise side of the ring
            // Is this the best choice? are there others?
            pkt->setHopCount(pkt->getHopCount() + 1);

            // Por defecto mandamos a la gate 0
            int gate = 0;
            for (int i = 0; i < netLength; i++){
                if (nodeIndexArray[i] == pkt->getDestination()){
                  // Checkeeamos si nos conviene mandar por la izqueirda o seguir madnando pr la derecha
                  if (i > netLength - i){
                      gate = 1;
                  }
                  break;
                }
            }
            LnkSelectVector.record(gate);
            send(msg, "toLnk$o", gate);
        }
    }
}
