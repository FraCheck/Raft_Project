#include "../../client_server/add_command_response.h"
#include "../append_entries/append_entries.h"
#include "../append_entries/append_entries_response.h"

void AppendEntriesResponse::handleOnServer(Server *server) const {
    int senderIndex = getArrivalGate()->getIndex();

    if (!result) {
        // "If AppendEntries fails because of log inconsistency:
        // decrement nextIndex and retry."

        server->nextIndex[senderIndex]--;

        int logEntryToSendIndex = server->nextIndex[senderIndex];
        LogEntry logEntry = server->log->getFromIndex(logEntryToSendIndex);
        LogEntry prevLogEntry = server->log->getFromIndex(
                logEntryToSendIndex - 1);

        AppendEntries *request = new AppendEntries("AppendEntries",
                server->currentTerm, server->getIndex(), prevLogEntry.index,
                prevLogEntry.term, { logEntry }, server->commitIndex);

        server->send(request, "out", senderIndex);
        return;
    }

    // "If successful: update nextIndex and matchIndex for follower."

    server->matchIndex[senderIndex] = lastLogIndex;
    server->nextIndex[senderIndex] = lastLogIndex + 1;

    server->logNextAndMatchIndexes();

    // Check if commitIndex should be updated

    int replicationCount = 0;
    for (int i = 0; i < server->getVectorSize(); i++) {
        if (server->matchIndex[i] >= lastLogIndex)
            replicationCount++;
    }

    EV << "Replication count for log " << lastLogIndex << " is "
              << replicationCount << endl;

    EV << "Old commitIndex" << server->commitIndex << endl;

    if (replicationCount > server->getVectorSize() / 2) {
        server->commitIndex = lastLogIndex;

        EV << "commitIndex updated to " << server->commitIndex << endl;
    }

    // "When the entry has been safely replicated, the leader applies
    // the entry to its state machine and returns the result of that
    // execution to the client."

    for (int logIndex = server->commitIndex; logIndex > 0; logIndex--) {
        LogEntry logEntry = server->log->getFromIndex(logIndex);
        if (logEntry.isCommitted)
            break;

        EV << "Entry " << logIndex << " isCommitted = " << logEntry.isCommitted
                  << endl;

        // Set entries as committed
        server->log->commit(logIndex);

        // Send to client response corresponding to new entry committed
        AddCommandResponse *response = new AddCommandResponse(true,
                server->getIndex(), logEntry.commandId);
        server->send(response, "toclients", logEntry.clientId);

    }
}
