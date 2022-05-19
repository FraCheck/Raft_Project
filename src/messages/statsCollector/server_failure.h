#ifndef MESSAGES_LEADER_FAILURE_H_
#define MESSAGES_LEADER_FAILURE_H_

#include <string>
#include "../handable_message.h"
#include "../../statsCollector.h"

class ServerFailure: public HandableMessage {

public:
    ServerFailure() {
        cMessage::setName("LeaderFailure");
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override {
        if (statsCollector->is_election_ongoing == false) {
            statsCollector->is_election_ongoing = true;
            statsCollector->server_failed = simTime();
        }
    }
    ;

    cMessage* dup() const override {
        return new ServerFailure();
    }
};

#endif
