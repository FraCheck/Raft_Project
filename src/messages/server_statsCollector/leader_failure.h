#ifndef MESSAGES_LEADER_FAILURE_H_
#define MESSAGES_LEADER_FAILURE_H_

#include <string>
#include "../handable_message.h"
#include "../../statsCollector.h"

class LeaderFailure: public HandableMessage {


public:
    LeaderFailure() {
        cMessage::setName("LeaderFailure");
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override;

    cMessage* dup() const override {
        return new LeaderFailure();
    }
};

#endif
