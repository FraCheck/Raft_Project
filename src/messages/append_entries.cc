#include <string.h>
#include <omnetpp.h>
#include <list>
#include <string>
#include "../utilityclasses/LogEntry.h"
using namespace omnetpp;
using namespace std;

class AppendEntries: public cMessage {
private:
    int term;
    int leaderId;
    int prevLogIndex;
    int prevLogTerm;
    list<LogEntry> entries;
    int  leaderCommit;
public:
    AppendEntries(string name,int term,int leaderId,int prevLogIndex, int prevLogTerm,list<LogEntry> entries,int leaderCommit)

       {
         this->setName(name.c_str());
         this->term=term;
         this->leaderId=leaderId;
         this->prevLogIndex=prevLogIndex;
         this->entries=entries;
         this->leaderCommit=leaderCommit;
       }
    list<LogEntry> getEntries() const {
        return entries;
    }

    void setEntries(list<LogEntry> entries) {
        this->entries = entries;
    }

    int getLeaderCommit() const {
        return leaderCommit;
    }

    void setLeaderCommit(int leaderCommit) {
        this->leaderCommit = leaderCommit;
    }

    int getLeaderId() const {
        return leaderId;
    }

    void setLeaderId(int leaderId) {
        this->leaderId = leaderId;
    }

    int getPrevLogIndex() const {
        return prevLogIndex;
    }

    void setPrevLogIndex(int prevLogIndex) {
        this->prevLogIndex = prevLogIndex;
    }

    int getPrevLogTerm() const {
        return prevLogTerm;
    }

    void setPrevLogTerm(int prevLogTerm) {
        this->prevLogTerm = prevLogTerm;
    }

    int getTerm() const {
        return term;
    }

    void setTerm(int term) {
        this->term = term;
    }
    cMessage* dup() const override {

            return new AppendEntries("AppendEntries",this->term,this->leaderId,this->prevLogIndex,this->prevLogTerm,this->entries,this->leaderCommit);
        }


};
