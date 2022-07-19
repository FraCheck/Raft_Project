#ifndef RECOVERING_SERVER_STATUS_H_
#define RECOVERING_SERVER_STATUS_H_

#include <omnetpp.h>
using namespace omnetpp;

class RecoveringServerStatus {

public:
   int server_index;
   bool isRecovering;
   simtime_t recovery_start_timestamp;
   int server_log_index;

   RecoveringServerStatus(int server_index, bool isRecovering, simtime_t recovery_start_timestamp, int server_log_index){
        this->server_index = server_index;
        this->isRecovering = isRecovering;
        this->recovery_start_timestamp = recovery_start_timestamp;
        this->server_log_index = server_log_index;
   }
};

#endif