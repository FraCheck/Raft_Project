#ifndef CLIENT_COMMAND_STATUS_H_
#define CLIENT_COMMAND_STATUS_H_

#include <omnetpp.h>
using namespace omnetpp;

class ClientCommandStatus {

public:
   int command_id;
   int messages_count_at_timestamp;
   simtime_t command_issued_messages_count_at_timestamp;
   

   ClientCommandStatus(int command_id, int messages_count_at_timestamp,  simtime_t command_issued_messages_count_at_timestamp){
        this->command_id = command_id;
        this->messages_count_at_timestamp = messages_count_at_timestamp;
        this->command_issued_messages_count_at_timestamp = command_issued_messages_count_at_timestamp;
   }
};

#endif