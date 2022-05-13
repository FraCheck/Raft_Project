#include "server_failure.h"

void ServerFailure::handleOnStatsCollector(StatsCollector *statsCollector) const {
    if (statsCollector->is_election_ongoing == false){
        statsCollector->is_election_ongoing = true;
        statsCollector->leader_failed = simTime();
    }
}
