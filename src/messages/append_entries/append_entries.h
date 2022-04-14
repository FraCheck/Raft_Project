#ifndef MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_H_
#define MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_H_

#include "../handable_message.h"

using namespace omnetpp;
using namespace std;

class AppendEntries: public HandableMessage {
private:
    // Leader’s term
    int term;

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
            int prevLogTerm, list<LogEntry> entries, int leaderCommit);

    void handleOnServer(Server *server) const override;
    cMessage* dup() const override;

    // Getter methods
    int getTerm() const;
    int getLeaderId() const;
    int getPrevLogIndex() const;
    int getPrevLogTerm() const;
    list<LogEntry> getEntries() const;
    int getLeaderCommit() const;

};

#endif
