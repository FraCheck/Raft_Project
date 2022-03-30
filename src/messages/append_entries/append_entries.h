#ifndef MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_H_
#define MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_H_

#include "../handable_message.h"

using namespace omnetpp;
using namespace std;

class AppendEntries: public HandableMessage {
private:
    int term;
    int leaderId;
    int prevLogIndex;
    int prevLogTerm;
    list<LogEntry> entries;
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
