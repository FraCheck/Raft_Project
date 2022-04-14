#include<iterator>
#include <list>
#include "../../rpc/append_entries/append_entries.h"
#include "../../rpc/append_entries/append_entries_response.h"
#include "../../../utils/log_entry.h"

void AppendEntries::handleOnServer(Server *server) const {
    if (server->currentState != LEADER)
        server->rescheduleElectionTimeout();

    // Manage HeartBeats
    if (entries.empty()) {
        if (server->currentState == CANDIDATE && term >= server->currentTerm) {
            // Accept the HeartBeat sender as Leader
            server->currentState = FOLLOWER;
            server->currentTerm = term;
            server->votesCount = 0;
        }

        server->currentLeader = leaderId;
        return;
    }

    // N.B: Entries are implemented as a list,
    // but actually there will be maximum 1 entry at a time

    if (server->log.size() >= this->prevLogIndex) {
        list<LogEntry>::iterator it = server->log.begin();

        if (prevLogIndex == 0) {
            // Server log is empty: add all received entries
            list<LogEntry> entries_ = entries;
            server->log.splice(server->log.end(), entries_);

            AppendEntriesResponse *response = new AppendEntriesResponse(
                    server->currentTerm, true, server->log.size());
            server->send(response, "out", getArrivalGate()->getIndex());
            return;
        }

        // Server log is not empty

        advance(it, prevLogIndex - 1);
        LogEntry logToCheck = *it;

        // Find the common entry with the leader
        if (logToCheck.getLogTerm() == term) { //if such entry exist...

            list<LogEntry>::iterator tail = server->log.begin();
            advance(tail, server->log.size());

            list<LogEntry> entries_ = entries; //why doesn' t work if i put directly entries in splice below?

            list<LogEntry>::iterator itrem = server->log.begin();
            if (prevLogIndex < server->log.size()) {
                advance(itrem, prevLogIndex);

                // Delete entries after the common one
                server->log.erase(itrem, tail);
            }

            // Add new entries
            server->log.splice(server->log.end(), entries_);

            // No common entry found
            AppendEntriesResponse *response = new AppendEntriesResponse(
                    server->currentTerm, true, server->log.size());
            server->send(response, "out", getArrivalGate()->getIndex());
            return;
        } else { // common entry not found
            server->send(
                    new AppendEntriesResponse(server->currentTerm, false,
                            server->log.size()), "out",
                    getArrivalGate()->getIndex()); // no common entry found with leader
            return;
        }

    } else { // no entry found at prevLogIndex
        server->send(
                new AppendEntriesResponse(server->currentTerm, false,
                        server->log.size()), "out",
                getArrivalGate()->getIndex());
        return;

    }

    //If leaderCommit > commitIndex, set commitIndex =   min(leaderCommit, index of last new entry)
    if (leaderCommit > server->commitIndex) {
        int indexoflastentry = server->log.size();
        if (indexoflastentry < leaderCommit)
            server->commitIndex = indexoflastentry;
        else
            server->commitIndex = leaderCommit;
    }
}

void AppendEntries::buildAndSendResponse(Server *server, bool success) const {
    AppendEntriesResponse *response = new AppendEntriesResponse(
            server->currentTerm, success, server->log.size());
    server->send(response, "out", getArrivalGate()->getIndex());
}

