#include <string.h>
#include <omnetpp.h>
#include <list>
#include <string>
#include "../utils/log_entry.h"
using namespace omnetpp;
using namespace std;

class AppendEntries: public cMessage {
private:
    int term;
    int leaderId;
    int prevLogIndex;
    int prevLogTerm;
    list<LogEntry> entries;
    int leaderCommit;
public:
    AppendEntries(string name, int term, int leaderId, int prevLogIndex,
            int prevLogTerm, list<LogEntry> entries, int leaderCommit) {
        this->setName(name.c_str());
        this->term = term;
        this->leaderId = leaderId;
        this->prevLogIndex = prevLogIndex;
        this->entries = entries;
        this->leaderCommit = leaderCommit;
    }

    list<LogEntry> getEntries() const {
        return entries;
    }

    int getLeaderCommit() const {
        return leaderCommit;
    }

    int getLeaderId() const {
        return leaderId;
    }

    int getPrevLogIndex() const {
        return prevLogIndex;
    }

    int getPrevLogTerm() const {
        return prevLogTerm;
    }

    int getTerm() const {
        return term;
    }

    cMessage* dup() const override {
        return new AppendEntries("AppendEntries", this->term, this->leaderId,
                this->prevLogIndex, this->prevLogTerm, this->entries,
                this->leaderCommit);
    }

};
