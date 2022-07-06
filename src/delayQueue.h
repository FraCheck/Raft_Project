#ifndef DELAY_H_
#define DELAY_H_

#include <omnetpp.h>
using namespace omnetpp;
using namespace std;

class DelayQueue: public cSimpleModule {
public:
    int numberOfServers;
    int numberOfClients;
    double avgServiceTime;
    bool isDelayQueueBusy;
    cMessage *msgInService;
    cMessage *endOfServiceMsg;
    cQueue queue;
    void startPacketService();
    void putPacketInQueue(cMessage *);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

};
Define_Module (DelayQueue);

#endif
