#include<iterator>
#include <list>
#include "../../rpc/append_entries/append_entries.h"
#include "../../rpc/append_entries/append_entries_response.h"
#include "../../../utils/log_entry.h"

void AppendEntries::handleOnServer(Server *server) const {
    // Check if it's an HeartBeat
    if (entries.empty()) {
        server->currentLeader = leaderId;
        return;
    }

    // N.B: Entries are implemented as a list,
    // but actually there will be maximum 1 entry at a time

    // Reject if there are not entries at prevLogIndex
    if (server->getLastLogIndex() < prevLogIndex) {
        buildAndSendResponse(server, false);
        return;
    }

    // Check the trivial case (server log is empty)
    if (prevLogIndex == 0) {

        // Why doesn't work if i put directly entries in splice below?
        list<LogEntry> entries_ = entries;
        server->log.splice(server->log.end(), entries_);

        buildAndSendResponse(server, true);
        return;
    }

    // Server log is NOT empty

    list<LogEntry>::iterator it = server->log.begin();
    advance(it, prevLogIndex - 1);
    LogEntry logToCheck = *it;

    // Find the common entry with the leader
    if (logToCheck.getLogTerm() == term) { //if such entry exist...

        list<LogEntry>::iterator tail = server->log.begin();
        advance(tail, server->log.size());

        list<LogEntry> entries_ = entries;

        list<LogEntry>::iterator itrem = server->log.begin();
        if (prevLogIndex < server->log.size()) {
            advance(itrem, prevLogIndex);

            // Delete entries after the common one
            server->log.erase(itrem, tail);
        }

        // Add new entries
        server->log.splice(server->log.end(), entries_);

        buildAndSendResponse(server, true);
        return;
    } else { // common entry not found
        buildAndSendResponse(server, false);
        return;
    }

    if (leaderCommit > server->commitIndex) {
        // Set commitIndex = min(leaderCommit, index of last new entry)
        int indexoflastentry = server->log.size();
        server->commitIndex =
                indexoflastentry < leaderCommit ?
                        indexoflastentry : leaderCommit;
    }
}

void AppendEntries::buildAndSendResponse(Server *server, bool success) const {
    AppendEntriesResponse *response = new AppendEntriesResponse(
            server->currentTerm, success, server->log.size());
    server->send(response, "out", getArrivalGate()->getIndex());
}

