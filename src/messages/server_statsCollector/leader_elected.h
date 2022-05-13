#ifndef MESSAGES_LEADER_ELECTED_H_
#define MESSAGES_LEADER_ELECTED_H_

#include <string>
#include "../handable_message.h"
#include "../../statsCollector.h"

class LeaderElected: public HandableMessage {


public:
    LeaderElected() {
        cMessage::setName("LeaderElected");
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override;

    cMessage* dup() const override {
        return new LeaderElected();
    }
};

#endif
