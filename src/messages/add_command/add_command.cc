#include "add_command.h"
#include "add_command_response.h"

#include "../rpc/append_entries/append_entries.h"

AddCommand::AddCommand(string command, int idrequest, int clientId) {
    cMessage::setName("AddCommand");
    this->command = command;
    this->requestId = idrequest;
    this->clientId = clientId;
}

void AddCommand::handleOnServer(Server *server) const {
    if (server->currentState != LEADER) {
        // Redirect the client to the current leader
        buildAndSendResponse(server, false);
        return;
    }

    // Look in the local log for a LogEntry with the same requestId
    for (int index = 0; index < server->log.size(); index++) {
        list<LogEntry>::iterator it = server->log.begin();
        advance(it, server->commitIndex - 1);
        LogEntry logToCheck = *it;
        if (logToCheck.getRequestId() == requestId) {

            // LogEntry found: reply to client
            if (logToCheck.isCommitted())
                buildAndSendResponse(server, true);
            return;
        }
    }

    // LogEntry not found: update the log
    LogEntry *newEntry = new LogEntry(server->currentTerm, command, requestId,
            clientId, server->log.size() + 1);
    server->log.push_back(*newEntry);

    int lastLogTerm;
    if (server->log.size() > 1) {
        list<LogEntry>::iterator prevLogIt = server->log.begin();
        advance(prevLogIt, server->log.size() - 2);
        lastLogTerm = (*prevLogIt).getLogTerm();
    } else
        lastLogTerm = 0;

    // Update followers
    AppendEntries *request = new AppendEntries("AppendEntries",
            server->currentTerm, server->getIndex(), server->log.size() - 1,
            lastLogTerm, { server->log.back() }, server->commitIndex);
    server->broadcast(request);

    server->cancelResendAppendEntries();
    server->scheduleResendAppendEntries();
}

void AddCommand::buildAndSendResponse(Server *server, bool result) const {
    AddCommandResponse *response = new AddCommandResponse(result,
            server->currentLeader, requestId);
    server->send(response, "toclients", getArrivalGate()->getIndex());
}

cMessage* AddCommand::dup() const {
    return new AddCommand(command, requestId, clientId);
}

