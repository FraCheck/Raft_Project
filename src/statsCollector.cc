#include <ctime>
#include <random>
#include "statsCollector.h"
#include "messages/handable_message.h"

void StatsCollector::initialize() {
    numberOfServers = getParentModule()->par("numServers");
    // Signals registering
    consensusTimeSignal = registerSignal("consensusTime");
    consensusMessagesSignal = registerSignal("consensusMessages");
    leader_failed = simTime();
    is_election_ongoing = true;
}

void StatsCollector::finish() {
}

void StatsCollector::handleMessage(cMessage *msg) {
    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {}
    // *** MESSAGES RECEIVED FROM SERVERS ***
    HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
    handableMsg->handleOnStatsCollector(this);
    cancelAndDelete(msg);
    return;
}

void StatsCollector::emitConsensusTime(){
    emit(consensusTimeSignal, new_leader_elected - leader_failed);
    EV << "[StatsCollector] Emitted time required to reach consensus: " << new_leader_elected - leader_failed << endl;
}

void StatsCollector::emitConsensunsMessges(){
    emit(consensusMessagesSignal, nb_messagesToConsensus);
     EV << "[StatsCollector] Emitted messages number required to reach consensus: " << nb_messagesToConsensus << endl;
}
