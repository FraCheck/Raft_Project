#ifndef MESSAGES_LEADER_ELECTED_H_
#define MESSAGES_LEADER_ELECTED_H_

#include <string>
#include "../handable_message.h"
#include "../../statsCollector.h"

class LeaderElected: public HandableMessage {
    int leader_term;

public:
    LeaderElected(int leader_term) {
        cMessage::setName("LeaderElected");
        this->leader_term = leader_term;
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override {
        if (statsCollector->is_election_ongoing == true) {
            statsCollector->is_election_ongoing = false;
            statsCollector->new_leader_elected = simTime();

            // Emit signals at the end of election
            statsCollector->emitConsensusTime();
            statsCollector->emitConsensunsMessges();

            // Restore messages exchanged count for next election
            statsCollector->nb_messagesToConsensus = 0;

            // Set the term of current leader
            statsCollector->leader_term = leader_term;
        }
    }
    ;

    cMessage* dup() const override {
        return new LeaderElected(leader_term);
    }
};

#endif
