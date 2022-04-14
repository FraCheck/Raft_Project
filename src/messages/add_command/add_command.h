#ifndef MESSAGES_ADD_COMMAND_H_
#define MESSAGES_ADD_COMMAND_H_

#include <string>
#include "../handable_message.h"
#include "../../server.h"

class AddCommand: public HandableMessage {
    // Command to be executed
    string command;

    int requestId; // Composed by clientId and request number
    int clientId;

public:
    AddCommand(string command, int idrequest, int clientId);
    void handleOnServer(Server *server) const override;
    cMessage* dup() const override;

private:
    void buildAndSendResponse(Server *server, bool result) const;
};

#endif
