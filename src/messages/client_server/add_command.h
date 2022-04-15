#ifndef MESSAGES_ADD_COMMAND_H_
#define MESSAGES_ADD_COMMAND_H_

#include <string>
#include "../handable_message.h"
#include "../../server.h"

class AddCommand: public HandableMessage {
    // Command to be added to the server log as LogEntry
    string command;

    // Composed by clientId and request number
    int requestId;

    int clientId;

public:
    AddCommand(string command, int requestId, int clientId) {
        cMessage::setName("AddCommand");
        this->command = command;
        this->requestId = requestId;
        this->clientId = clientId;
    }

    void handleOnServer(Server *server) const override;

    cMessage* dup() const override {
        return new AddCommand(command, requestId, clientId);
    }

private:
    void buildAndSendResponse(Server *server, bool result) const;
};

#endif
