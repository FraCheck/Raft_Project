#ifndef STATSCOLLECTOR_H_
#define STATSCOLLECTOR_H_

#include <omnetpp.h>
#include <string>
#include <vector>
#include "utils/recovering_server_status.h"

using namespace omnetpp;
using namespace std;

class StatsCollector: public cSimpleModule {
public:
    int numberOfServers;
    simtime_t leader_failed, new_leader_elected;
    int nb_messagesToConsensus = 0;
    bool is_election_ongoing;
    int currentLeader = -1;
    int leader_term;
    int leader_last_log_index;
    std::vector<RecoveringServerStatus*> recoveryServersStatus;
    // SIGNALS
    simsignal_t consensusTimeSignal;
    simsignal_t consensusMessagesSignal;
    simsignal_t timeToRecoverLogSignal;

    void emitConsensusTime();
    void emitTimeToRecoverLog(simtime_t time_to_update_log, int server_index);
    void emitConsensunsMessges();
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(StatsCollector);

#endif
