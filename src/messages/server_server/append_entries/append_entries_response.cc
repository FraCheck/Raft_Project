#include "../../client_server/add_command_response.h"
#include "../append_entries/append_entries.h"
#include "../append_entries/append_entries_response.h"
#include "../../statsCollector/server_log_update.h"

void AppendEntriesResponse::handleOnServer(Server *server) const {
    int senderIndex = getArrivalGate()->getIndex();
    if(!successterm) return;
    if (!result) {
        // "If AppendEntries fails because of log inconsistency:
        // decrement nextIndex and retry."
        server->nextIndex[senderIndex]--;
        buildAndSendNextAppendEntriesRequest(server, senderIndex);
        return;
    }

    // "If successful: update nextIndex and matchIndex for follower."


        server->matchIndex[senderIndex] = lastLogIndex;
     if(lastLogIndex < server->log->size() ){
    server->nextIndex[senderIndex] = lastLogIndex + 1;
     }
     else {
         server->nextIndex[senderIndex] = server->log->size() +1 ;
     }

    // Check if the sender needs other entries to be consistent
    if (lastLogIndex < server->getLastLogIndex() && lastLogIndex != 0)
        buildAndSendNextAppendEntriesRequest(server, senderIndex);

    // Check if commitIndex should be updated

    int replicationCount = 0;
    for (int i = 0; i < server->getVectorSize(); i++) {
        if (server->matchIndex[i] >= lastLogIndex)
            replicationCount++;
    }

    if (replicationCount > server->getVectorSize() / 2) {
        server->commitIndex = lastLogIndex;
    }

    // "When the entry has been safely replicated, the leader applies
    // the entry to its state machine and returns the result of that
    // execution to the client."

    for (int logIndex = (server->commitIndex > server->log->size() ? server->log->size() : server->commitIndex); logIndex > 0; logIndex--) {
        LogEntry logEntry = server->log->getFromIndex(logIndex);
        if (logEntry.isCommitted)
            break;

        // Set entries as committed
        server->log->commit(logIndex);

        // Send to client response corresponding to new entry committed
        AddCommandResponse *response = new AddCommandResponse(true,
                server->getParentModule()->getIndex(), logEntry.commandId);
        server->send(response, "toclients", logEntry.clientId);
        if (!(server->getParentModule()->getParentModule()->par("disableStatsCollector"))){
            // Update the StatsCollector with the current last commited index of the leader
            ServerLogUpdate *serverLogUpdate = new ServerLogUpdate(server->getParentModule()->getIndex(), logIndex, logEntry.commandId);
            server->sendToStatsCollector(serverLogUpdate);
        }
    }
}

void AppendEntriesResponse::buildAndSendNextAppendEntriesRequest(Server *server,
        int senderIndex) const {
    //        int logEntryToSendIndex = server->matchIndex[senderIndex] + 1;
    if(server->nextIndex[senderIndex]<1) server->nextIndex[senderIndex]=1;
    if(server->nextIndex[senderIndex]>server->log->size()) server->nextIndex[senderIndex]=server->log->size();
    int logEntryToSendIndex = server->nextIndex[senderIndex];



    LogEntry logEntry = server->log->getFromIndex(logEntryToSendIndex);


    int prevlogentryindex;
    int prevlogentryterm;
    if(logEntryToSendIndex>1){
        LogEntry  prevLogEntry = server->log->getFromIndex(logEntryToSendIndex - 1);
        prevlogentryindex = prevLogEntry.index;
        prevlogentryterm = prevLogEntry.term;
    }
    else{
        prevlogentryindex = 0;
        prevlogentryterm = 0;
    }

    AppendEntries *request = new AppendEntries("AppendEntries",
            server->currentTerm, server->getParentModule()->getIndex(), prevlogentryindex,
            prevlogentryterm, { logEntry }, server->commitIndex);

    server->send(request, "out", senderIndex);
}
