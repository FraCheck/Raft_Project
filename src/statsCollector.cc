#include "statsCollector.h"
#include "messages/handable_message.h"
#include "messages/client_server/add_command.h"


void StatsCollector::initialize() {
    numberOfServers = getParentModule()->par("numServers");
    // Signals registering
    consensusTimeSignal = registerSignal("consensusTime");
    timeToRecoverLogSignal = registerSignal("timeToRecoverLog");
    consensusMessagesSignal = registerSignal("consensusMessages");
    commitMessagesSignal = registerSignal("commitMessages");
    commandResponseTimeSignal = registerSignal("commandResponseTime");

    leader_failed = simTime();
    is_election_ongoing = false;
    leader_term = 0;

    for (int i=0; i<numberOfServers; i++){
        RecoveringServerStatus *recoveringServerStatus = new RecoveringServerStatus(i, false, 0, 0);
        recoveryServersStatus.push_back(recoveringServerStatus);
    }
    
}

void StatsCollector::finish() {
    recoveryServersStatus.clear();
    clientCommandsStatus.clear();
}

void StatsCollector::handleMessage(cMessage *msg) {
    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {}
    // *** MESSAGES RECEIVED FROM SERVERS ***

    if (dynamic_cast<AddCommand*>(msg) != nullptr){
        int command_id = check_and_cast<AddCommand *>(msg)->getCommandId();
        EV << "[StatsCollector] Received Command[" << command_id << "], added it to the list." << endl;
        ClientCommandStatus *clientCommandStatus = new ClientCommandStatus(command_id, exchanged_messages, simTime());
        clientCommandsStatus.push_back(clientCommandStatus);
       
        string list = getMonitoredCommands();
        EV << "New commands list being monitored: " << list << endl;

        cancelAndDelete(msg);
        return;

    }

    HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
    handableMsg->handleOnStatsCollector(this);
    cancelAndDelete(msg);
    return;
}

void StatsCollector::increase_exchanged_messages(){
    exchanged_messages++;
}

void StatsCollector::committedEntry(int command_id){
    if (clientCommandsStatus.size() == 0)
        return;

    for (int i = 0; i < clientCommandsStatus.size(); i++){
        if (clientCommandsStatus[i]->command_id == command_id){
            emitCommittedMessages(command_id, exchanged_messages - clientCommandsStatus[i]->messages_count_at_timestamp, 
                clientCommandsStatus[i]->command_issued_messages_count_at_timestamp);
            clientCommandsStatus.erase(clientCommandsStatus.begin()+i);
            EV << "[StatsCollector] Removed Command[" << command_id << "] from the list." << endl;
            string list = getMonitoredCommands();
            EV << "New commands list being monitored: " << list << endl;

            if(clientCommandsStatus.size() == 0)
                exchanged_messages = 0;
                
            return;
        }
        
    }
    EV << "[StatsCollector] Received a log commit for an AddCommand never seen" << endl;
}

void StatsCollector::consensusMessagesIncrement(int nb_messages){
    if (this->is_election_ongoing){
        this->nb_messagesToConsensus+=nb_messages;
        EV << "[StatsCollector] Incremented the count of messages exchanged during election by " <<
            nb_messages << "." << endl;
    }
}


void StatsCollector::emitConsensusTime(){
    emit(consensusTimeSignal, new_leader_elected - leader_failed);
    EV << "[StatsCollector] Emitted time required to reach consensus: " << new_leader_elected - leader_failed << endl;
}

void StatsCollector::emitConsensunsMessges(){
    emit(consensusMessagesSignal, nb_messagesToConsensus);
     EV << "[StatsCollector] Emitted messages number required to reach consensus: " << nb_messagesToConsensus << endl;
}

void StatsCollector::emitTimeToRecoverLog(simtime_t time_to_update_log, int server_index){
    emit(timeToRecoverLogSignal, time_to_update_log);
    EV << "[StatsCollector] Emitted time required to get up to date for Server[" << server_index <<"]: " << time_to_update_log << endl;
}

void StatsCollector::emitCommittedMessages(int command_id, int messages_exchanged_to_commit, 
    simtime_t command_issued_messages_count_at_timestamp){
    emit(commitMessagesSignal, messages_exchanged_to_commit);
    EV << "[StatsCollector] Emitted messages number required to commit a command: " << messages_exchanged_to_commit <<
        " messages were required to commit Command[" << command_id << "] in " << simTime() - command_issued_messages_count_at_timestamp 
            << endl;
}

void StatsCollector::emitCommandTimeResponse(simtime_t time, int client_id){
    emit(commandResponseTimeSignal, time);
    EV << "[StatsCollector] Emitted command execution response time for Client[" << client_id << "]: " << time << endl;
}

string StatsCollector::getMonitoredCommands(){
    if (clientCommandsStatus.size() == 0)
        return "[]";
    string list = "[ ";
    for (int i = 0; i < clientCommandsStatus.size(); i++){
        list = list + to_string(clientCommandsStatus[i]->command_id) + " ";
    }
    list = list + "]";
    return list;
}

