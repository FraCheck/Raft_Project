#ifndef MESSAGES_SERVER_RECOVERING_H_
#define MESSAGES_SERVER_RECOVERING_H_

#include "../handable_message.h"
#include "../../statsCollector.h"

class ServerRecovering: public HandableMessage {
    int serverIndex;
    int server_log_index;
public:
    ServerRecovering(int serverIndex,int server_log_index) {
        cMessage::setName("ServerRecovering");
        this->serverIndex = serverIndex;
        this->server_log_index = server_log_index;
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override {
        if ((serverIndex < 0) || (serverIndex > (statsCollector->numberOfServers)-1))
            throw invalid_argument("Index Out Of Bound: " + to_string(serverIndex));
        if (serverIndex != statsCollector->recoveryServersStatus[serverIndex]->server_index)
            throw invalid_argument("serverIndex from server and in recoveryServersStatus should be the same.");
        if (server_log_index == statsCollector->leader_last_log_index){
            // This server does not need any update
            EV << "[StatsCollector] : Server[" << serverIndex << "] does not need any update."<< endl;
            return;
        }
        if (statsCollector->recoveryServersStatus[serverIndex]->isRecovering == false){
            statsCollector->recoveryServersStatus[serverIndex]->isRecovering = true;
            statsCollector->recoveryServersStatus[serverIndex]->recovery_start_timestamp = simTime();
            statsCollector->recoveryServersStatus[serverIndex]->server_log_index = server_log_index;
            EV << "[StatsCollector] : Started counting recovery time for Server[" << serverIndex << "]."<< endl;
        }else{
            EV << "[StatsCollector] : Server[" << serverIndex << "] is still recovering from last crash." << endl;
        }
    }
    ;

    cMessage* dup() const override {
        return new ServerRecovering(this->serverIndex, this->server_log_index);
    }
};

#endif
