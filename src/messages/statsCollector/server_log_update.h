#ifndef SERVER_LOG_UPDATE_H_
#define SERVER_LOG_UPDATE_H_

#include "../handable_message.h"
#include "../../statsCollector.h"

class ServerLogUpdate: public HandableMessage {
    int serverIndex;
    int server_log_index;
public:
    ServerLogUpdate(int serverIndex,int server_log_index) {
        cMessage::setName("ServerLogUpdate");
        this->serverIndex = serverIndex;
        this->server_log_index = server_log_index;
    }

    void handleOnStatsCollector(StatsCollector *statsCollector) const override {
       if (serverIndex == statsCollector->currentLeader){
            // log update commited for the leader
            statsCollector->leader_last_log_index = server_log_index;
       }else{
            // log update from a server
            if ((serverIndex < 0) || (serverIndex > (statsCollector->numberOfServers)-1))
                throw invalid_argument("Index Out Of Bound: " + to_string(serverIndex));
            if (serverIndex != statsCollector->recoveryServersStatus[serverIndex]->server_index)
                throw invalid_argument("serverIndex from server and in recoveryServersStatus should be the same.");
            if (statsCollector->recoveryServersStatus[serverIndex]->isRecovering == true){
                statsCollector->recoveryServersStatus[serverIndex]->server_log_index = server_log_index;

                if (server_log_index >= statsCollector->leader_last_log_index){
                    EV << "server_log_index = " << server_log_index << endl;
                    EV << "leader_last_log_index = " << statsCollector->leader_last_log_index << endl;
                    // Server is now up to date
                    statsCollector->recoveryServersStatus[serverIndex]->isRecovering = false;
                    statsCollector->emitTimeToRecoverLog(simTime() - statsCollector->recoveryServersStatus[serverIndex]->recovery_start_timestamp,serverIndex);
                }

            }

       }

    }
    ;

    cMessage* dup() const override {
        return new ServerLogUpdate(this->serverIndex, this->server_log_index);
    }
};

#endif
