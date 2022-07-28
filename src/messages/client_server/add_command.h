#ifndef MESSAGES_ADD_COMMAND_H_
#define MESSAGES_ADD_COMMAND_H_

#include <string>
#include "../handable_message.h"
#include "../../server.h"

class AddCommand: public HandableMessage {
    // Command to be added to the server log as LogEntry
    string command;

    // Composed by clientId and request number
    int commandId;

    int clientId;

public:
    AddCommand(int commandId, string command, int clientId) {
        char messageName[32];
        sprintf(messageName, " %d",commandId);
        cMessage::setName(messageName);
        this->commandId = commandId;
        this->command = command;
        this->clientId = clientId;
    }

    void handleOnServer(Server *server) const override;
    int getCommandId();

    cMessage* dup() const override {
        return new AddCommand(commandId, command, clientId);
    }

private:
    void buildAndSendResponse(Server *server, bool result) const;
};

#endif
