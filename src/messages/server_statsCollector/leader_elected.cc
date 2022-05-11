#include "../server_statsCollector/leader_elected.h"

void LeaderElected::handleOnStatsCollector(StatsCollector *statsCollector) const {
    if (statsCollector->is_election_ongoing == true){
        statsCollector->is_election_ongoing = false;
        statsCollector->new_leader_elected = simTime();
        statsCollector->emitConsensusTime();
    }

}
