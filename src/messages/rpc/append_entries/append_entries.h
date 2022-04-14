#ifndef MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_H_
#define MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_H_

#include "../../handable_message.h"
#include "../rpc.h"

using namespace omnetpp;
using namespace std;

class AppendEntries: public HandableMessage, public RPCRequest {
private:
    // Used by followers to redirect clients
    int leaderId;

    // Index of the LogEntry immediately preceding the new ones
    int prevLogIndex;

    // Term of prevLogIndex entry
    int prevLogTerm;

    // LogEntries to store (empty for HeartBeat;
    // may send more than one for efficiency)
    list<LogEntry> entries;

    // Leader’s commitIndex
    int leaderCommit;

public:
    AppendEntries(string name, int term, int leaderId, int prevLogIndex,
            int prevLogTerm, list<LogEntry> entries, int leaderCommit) :
            RPCRequest(term) {
        cMessage::setName(name.c_str());
        this->leaderId = leaderId;
        this->prevLogIndex = prevLogIndex;
        this->prevLogTerm = prevLogTerm;
        this->entries = entries;
        this->leaderCommit = leaderCommit;
    }

    void handleOnServer(Server *server) const override;
    void buildAndSendResponse(Server *server, bool success) const override;

    cMessage* dup() const override {
        return new AppendEntries(getName(), term, leaderId, prevLogIndex,
                prevLogTerm, entries, leaderCommit);
    }
};

#endif
