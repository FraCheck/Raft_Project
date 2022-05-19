#ifndef MESSAGES_LEADER_ELECTED_H_
#define MESSAGES_LEADER_ELECTED_H_

#include <string>
#include "../handable_message.h"
#include "../../statsCollector.h"

class CommandSent: public HandableMessage {

public:
    CommandSent() {
        cMessage::setName("LeaderElected");
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override {
        if (statsCollector->is_election_ongoing == true) {
            statsCollector->is_election_ongoing = false;
            statsCollector->new_leader_elected = simTime();
            statsCollector->emitConsensusTime();
        }
    }
    ;

    cMessage* dup() const override {
        return new CommandSent();
    }
};

#endif
