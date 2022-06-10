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
    string lastCommand;

    simsignal_t commandResponseTimeSignal;
    simtime_t commandTimestamp;

    void scheduleSendCommand();
    void cancelSendCommandTimeout();
    void scheduleResendCommand();
    void cancelResendCommandTimeout();
    void emitCommandTimeResponseSignal();

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

Define_Module (Client);

#endif
