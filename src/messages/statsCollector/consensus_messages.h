#ifndef MESSAGES_CONSENSUS_MSG_H_
#define MESSAGES_CONSENSUS_MSG_H_

#include <string>
#include "../handable_message.h"
#include "../../statsCollector.h"

class ConsensusMessages: public HandableMessage {
    int nbOfMessages;
public:
    ConsensusMessages(int nbOfMessages = 1) {
        cMessage::setName("ConsensusMessages");
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override {
        if (statsCollector->is_election_ongoing == true) {
            statsCollector->nb_messagesToConsensus++;
        }
    }
    ;

    cMessage* dup() const override {
        return new ConsensusMessages();
    }
};

#endif
