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

    void handleOnStatsCollector(StatsCollector *statsCollector) const override;

    cMessage* dup() const override {
        return new ServerFailure();
    }
};

#endif
