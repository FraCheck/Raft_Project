#include <ctime>
#include <random>
#include "statsCollector.h"
#include "messages/handable_message.h"

void StatsCollector::initialize() {
    numberOfServers = par("numServers");
    // Signals registering
    consensusTimeSignal = registerSignal("consensusTime");
    server_failed = simTime();
    is_election_ongoing = true;
}

void StatsCollector::finish() {
}

void StatsCollector::handleMessage(cMessage *msg) {
    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {

    }
    // *** MESSAGES RECEIVED FROM SERVERS ***
    HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
    handableMsg->handleOnStatsCollector(this);
}

void StatsCollector::emitConsensusTime(){
    emit(consensusTimeSignal, new_leader_elected - server_failed);
}