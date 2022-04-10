

#ifndef MESSAGES_CLIENTSENDCOMMAND_H_
#define MESSAGES_CLIENTSENDCOMMAND_H_
#include <string>
#include "../server.h"
#include "handable_message.h"
class ClientSendCommand : public HandableMessage {
string command; //command to be executed from server side
int idrequest; // id of request composed of client id and number of request
int clientId ;


public:
    ClientSendCommand(string command, int idrequest,int clientId);
    void handleOnServer(Server* server) const override;
        cMessage* dup() const override;
};

#endif /* MESSAGES_CLIENTSENDCOMMAND_H_ */
