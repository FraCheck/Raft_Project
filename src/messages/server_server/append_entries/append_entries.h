#ifndef MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_H_
#define MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_H_

#include "../../handable_message.h"
#include "../../server_server/rpc.h"

using namespace omnetpp;
using namespace std;

class AppendEntries: public HandableMessage, public RPCRequest {
public:
    // Used by followers to redirect clients
    int leaderId;

    // Index of the LogEntry immediately preceding the new ones
    int prevLogIndex;

    // Term of the LogEntry immediately preceding the new ones
    int prevLogTerm;

    // LogEntries to store (empty for HeartBeat;
    // may send more than one for efficiency)
    vector<LogEntry> entries;

    // Leader’s commitIndex
    int leaderCommit;

public:
    AppendEntries(string name, int term, int leaderId, int prevLogIndex,
            int prevLogTerm, vector<LogEntry> entries, int leaderCommit) :
            RPCRequest(term) {
        if (!entries.empty())
            cMessage::setName(to_string(entries[0].index).c_str());
        else
            cMessage::setName("Heartbeat");
        this->leaderId = leaderId;
        this->prevLogIndex = prevLogIndex;
        this->prevLogTerm = prevLogTerm;
        this->entries = entries;
        this->leaderCommit = leaderCommit;
    }

    void handleOnServer(Server *server) const override;
    void buildAndSendResponse(Server *server, bool success) const override;

    const char* getDisplayString() const override {
        return "b=14,14,oval,yellow,black,1.2;t={}";
    }

    cMessage* dup() const override {
        return new AppendEntries(getName(), term, leaderId, prevLogIndex,
                prevLogTerm, entries, leaderCommit);
    }
};

#endif
