#include "../client_server/add_command.h"
#include "../client_server/add_command_response.h"
#include "../server_server/append_entries/append_entries.h"

void AddCommand::handleOnServer(Server *server) const {
    if (server->state != LEADER) {
        // Redirect the client to the current leader
        buildAndSendResponse(server, false);
        return;
    }

    // "The leader appends the command to its log as a new entry,
    // then issues AppendEntries RPCs in parallel to each of the other
    // servers to replicate the entry."

    // Check if the command has already been received and is committed
    for (int index = 1; index <= server->log->size(); index++) {
        LogEntry logEntry = server->log->getFromIndex(index);

        if (logEntry.commandId == commandId) {
            if (logEntry.isCommitted)
                buildAndSendResponse(server, true);

            cout << "LogEntry already stored: return";
            return;
        }
    }

    // LogEntry not yet stored: update the log
    LogEntry *newEntry = new LogEntry(server->currentTerm, command, commandId,
            clientId, server->getLastLogIndex() + 1);
    server->log->append(newEntry);

    // Send AppendEntries RPCs to followers
    int prevLogEntryIndex;
    int prevLogEntryTerm;
    try {
        LogEntry prevLogEntry = server->log->getSecondToLast();
        prevLogEntryIndex = prevLogEntry.index;
        prevLogEntryTerm = prevLogEntry.term;
    } catch (out_of_range e) {
        prevLogEntryIndex = 0;
        prevLogEntryTerm = 0;
    }
    AppendEntries *request = new AppendEntries("AppendEntries",
            server->currentTerm, server->getIndex(), prevLogEntryIndex,
            prevLogEntryTerm, { newEntry }, server->commitIndex);
    for (int s = 0; s < server->gateSize("out"); s++)
        if (server->nextIndex[s] == server->getLastLogIndex())
            server->send(request->dup(), "out", s);
}

void AddCommand::buildAndSendResponse(Server *server, bool result) const {
    AddCommandResponse *response = new AddCommandResponse(result,
            server->currentLeader, commandId);
    server->send(response, "toclients", getArrivalGate()->getIndex());
}

