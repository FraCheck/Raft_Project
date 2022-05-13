#ifndef STATSCOLLECTOR_H_
#define STATSCOLLECTOR_H_

#include <omnetpp.h>
#include <string>

using namespace omnetpp;
using namespace std;

//
class StatsCollector: public cSimpleModule {
public:
    int numberOfServers;
    simtime_t server_failed, new_leader_elected;
    bool is_election_ongoing;

    // SIGNALS
    simsignal_t consensusTimeSignal;

    void emitConsensusTime();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(StatsCollector);

#endif
