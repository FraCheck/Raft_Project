#ifndef MESSAGES_LEADER_FAILURE_H_
#define MESSAGES_LEADER_FAILURE_H_

#include <string>
#include "../handable_message.h"
#include "../../statsCollector.h"

class ServerFailure: public HandableMessage {
    bool isLeader;
    int server_term;
public:
    ServerFailure(bool isLeader = false, int server_term = 0) {
        cMessage::setName("ServerFailure");
        this->isLeader = isLeader;
        this->server_term = server_term;
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override {
        if (isLeader){
            // the request of the server will not produce a new leader for the term
            if (server_term <= statsCollector->leader_term){
                EV << "There will be no election because a leader already exists in a more recent term" << endl;
                return;
            }
            if (statsCollector->is_election_ongoing == false) {
                statsCollector->is_election_ongoing = true;
                statsCollector->leader_failed = simTime();
                statsCollector->consensusMessagesIncrement(statsCollector->numberOfServers - 1);
                EV << "[StatsCollector] - Leader crashed: elections metrics are now monitored." << endl;
            }
        }
    }
    ;

    cMessage* dup() const override {
        return new ServerFailure(isLeader,server_term);
    }
};

#endif
