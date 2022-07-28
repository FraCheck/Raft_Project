#include<iterator>
#include <list>
#include "../../server_server/append_entries/append_entries.h"
#include "../../server_server/append_entries/append_entries_response.h"
#include "../../statsCollector/server_log_update.h"
#include "../../../utils/log_entry.h"

void AppendEntries::handleOnServer(Server *server) const {
    server->currentLeader = leaderId;
        if (server->state == CANDIDATE) {
            // Accept the HeartBeat sender as Leader
            server->state = FOLLOWER;
            server->votesCount = 0;
        }
    // "Reply false if log doesn't contain an entry at prevLogIndex
    // whose term matches prevLogTerm"

    if (server->getLastLogIndex() < prevLogIndex) {
        buildAndSendResponse(server, false);
        return;
    }

    // Check the trivial cases

    // 1. follower log is empty
    if (server->getLastLogIndex() == 0)
        goto appendEntries;

    // 2. follower log should be replaced
    if (prevLogIndex == 0) {
        server->log->eraseStartingFromIndex(1);
        goto appendEntries;
    }

    // Check if the previous LogEntry is the same
    if (server->log->getFromIndex(prevLogIndex).term != prevLogTerm) {
        buildAndSendResponse(server, false);
        return;
    }

    // "If an existing entry conflicts with a new one (same index
    // but different terms), delete the existing entry and all that
    // follow it"

    for (int logIndex = 1; logIndex <= server->log->size(); logIndex++)
        for (int i = 0; i < entries.size(); i++)
            if (server->log->getFromIndex(logIndex).index == entries[i].index
                    && server->log->getFromIndex(logIndex).term
                            != entries[i].term) {
                server->log->eraseStartingFromIndex(logIndex);
                break;
            }

    // "Append any new entries not already in the log"

    appendEntries:

    for (int i = 0; i < entries.size(); i++) {
        bool alreadyAppended = false;
        for (int logIndex = 1; logIndex <= server->log->size(); logIndex++)
            if (server->log->getFromIndex(logIndex).index == entries[i].index) {
                alreadyAppended = true;
                break;
            }

        if (!alreadyAppended){
            server->log->append(entries[i]);
            if (!(server->getParentModule()->getParentModule()->par("disableStatsCollector"))){
                ServerLogUpdate *serverLogUpdate = new ServerLogUpdate(server->getParentModule()->getIndex(), server->getLastLogIndex(),entries[i].commandId);
                server->sendToStatsCollector(serverLogUpdate);
            }
        }
        
    }

    // "If leaderCommit > commitIndex,
    // set commitIndex = min(leaderCommit, index of last new entry)"

    if (leaderCommit > server->commitIndex) {
        int lastLogIndex = server->getLastLogIndex();
        server->commitIndex =
                lastLogIndex < leaderCommit ? lastLogIndex : leaderCommit;
    }
    for(int indexlog=server->commitIndex;indexlog>0;indexlog--){
        server->log->commit(indexlog);
    }

    buildAndSendResponse(server, true);
}

void AppendEntries::buildAndSendResponse(Server *server, bool success) const {
    //EV << "SERVER " << server->getParentModule()->getIndex() << " is sending out appendResponse to queue: " << getArrivalGate()->getName() << "__" << getArrivalGate()->getIndex() << endl;
    AppendEntriesResponse *response = new AppendEntriesResponse(
            server->currentTerm, true, success, server->getLastLogIndex());
    server->send(response, "out", getArrivalGate()->getIndex());
}

