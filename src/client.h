#ifndef CLIENT_H_
#define CLIENT_H_

#include <omnetpp.h>
#include <string>
using namespace omnetpp;
using namespace std;

// Client sends commands, waits for a reply, sends commands, waits for a reply...
class Client: public cSimpleModule {
public:
    int numberOfServers;
    int lastCommandId;
    int numberOfRequests;
    string lastCommand;

    void scheduleSendCommand();
    void scheduleResendCommand();
    void cancelResendCommandTimeout();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

private:
    double sendCommandPeriod;
    double resendCommandPeriod;

    cMessage *sendCommandEvent;
    cMessage *resendCommandEvent;

    string buildRandomString(int);
};

Define_Module(Client);

#endif
