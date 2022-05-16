#include "../client_server/add_command_response.h"

#include "../client_server/add_command.h"

AddCommandResponse::AddCommandResponse(bool success, int leaderId,
        int requestId) {
    cMessage::setName("AddCommandResponse");
    this->success = success;
    this->leaderId = leaderId;
    this->requestId = requestId;
}

void AddCommandResponse::handleOnClient(Client *client) const {
    // Response received: cancel re-send timeout
    client->cancelResendCommandTimeout();

    if (client->lastCommandId != requestId) {
        client->bubble("Received response for another request");
        return;
    }

    if (!success) {
        // The message has been sent to the wrong server:
        // send the command to the leader
        AddCommand *request = new AddCommand(client->lastCommandId,
                client->lastCommand, client->getIndex());
        client->send(request, "out", leaderId);

        // Set a timeout to retry the request if the server is not responding
        client->scheduleResendCommand();
        return;
    }

    client->emitCommandTimeResponseSignal();
    // Schedule the sending of a new command
    client->scheduleSendCommand();
}

cMessage* AddCommandResponse::dup() const {
    return new AddCommandResponse(success, leaderId, requestId);
}