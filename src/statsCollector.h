#ifndef STATSCOLLECTOR_H_
#define STATSCOLLECTOR_H_

#include <omnetpp.h>
#include <string>
#include <vector>
#include "utils/recovering_server_status.h"
#include "utils/clients_command_status.h"
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
    int exchanged_messages = 0;
    std::vector<RecoveringServerStatus*> recoveryServersStatus;
    std::vector<ClientCommandStatus*> clientCommandsStatus;
    // SIGNALS
    simsignal_t consensusTimeSignal;
    simsignal_t consensusMessagesSignal;
    simsignal_t timeToRecoverLogSignal;
    simsignal_t commitMessagesSignal;
    simsignal_t commandResponseTimeSignal;

    void emitConsensusTime();
    void emitTimeToRecoverLog(simtime_t time_to_update_log, int server_index);
    void emitConsensunsMessges();
    void emitCommittedMessages(int command_id, int messages_exchanged_to_commit, simtime_t command_issued_messages_count_at_timestamp);
    void emitCommandTimeResponse(simtime_t time, int client_id);
    void increase_exchanged_messages();
    void committedEntry(int command_id);
    void consensusMessagesIncrement(int nb_messages = 1);
    string getMonitoredCommands();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(StatsCollector);

#endif
