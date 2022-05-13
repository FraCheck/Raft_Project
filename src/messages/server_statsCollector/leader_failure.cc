#include "../server_statsCollector/leader_failure.h"

void LeaderFailure::handleOnStatsCollector(StatsCollector *statsCollector) const {
    if (statsCollector->is_election_ongoing == false){
        statsCollector->is_election_ongoing = true;
        statsCollector->leader_failed = simTime();
    }
}
