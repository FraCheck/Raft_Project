#include "append_entries.h"

AppendEntries::AppendEntries(string name, int term, int leaderId,
        int prevLogIndex, int prevLogTerm, list<LogEntry> entries,
        int leaderCommit) {
    cMessage::setName(name.c_str());
    this->term = term;
    this->leaderId = leaderId;
    this->prevLogIndex = prevLogIndex;
    this->entries = entries;
    this->leaderCommit = leaderCommit;
}

void AppendEntries::handleOnServer(Server *server) const {
    if (entries.empty()) {
        // Received an heartbeat while being Follower: reschedule electionTimeout
        if (server->currentState == FOLLOWER)
            server->rescheduleElectionTimeout();

        // Received an heartbeat while being Candidate: compare the terms
        // and accept heartbeat sender as Leader if its term > my currentTerm
        if (server->currentState == CANDIDATE && term >= server->currentTerm) {
            server->currentState = FOLLOWER;
            server->currentTerm = term;
            server->votesCount = 0;
            server->votedFor = leaderId; // Update who the server voted for with the recognized Leader
        }
    }

    // TODO: implement reaction if the message has entries (not an heartbeat)

}

cMessage* AppendEntries::dup() const {
    return new AppendEntries(getName(), term, leaderId, prevLogIndex,
            prevLogTerm, entries, leaderCommit);
}

int AppendEntries::getTerm() const {
    return term;
}

int AppendEntries::getLeaderId() const {
    return leaderId;
}

int AppendEntries::getPrevLogIndex() const {
    return prevLogIndex;
}

int AppendEntries::getPrevLogTerm() const {
    return prevLogTerm;
}

list<LogEntry> AppendEntries::getEntries() const {
    return entries;
}

int AppendEntries::getLeaderCommit() const {
    return leaderCommit;
}
