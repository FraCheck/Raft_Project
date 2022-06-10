#ifndef MESSAGES_LEADER_FAILURE_H_
#define MESSAGES_LEADER_FAILURE_H_

#include <string>
#include "../handable_message.h"
#include "../../statsCollector.h"

class ServerFailure: public HandableMessage {
    bool isLeader;
public:
    ServerFailure(bool isLeader = false) {
        cMessage::setName("ServerFailure");
        this->isLeader = isLeader;
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override {
        if (isLeader){
            if (statsCollector->is_election_ongoing == false) {
                statsCollector->is_election_ongoing = true;
                statsCollector->leader_failed = simTime();
                EV << "[StatsCollector] - Leader crashed: elections metrics are now monitored.";
            }
        }
    }
    ;

    cMessage* dup() const override {
        return new ServerFailure();
    }
};

#endif
