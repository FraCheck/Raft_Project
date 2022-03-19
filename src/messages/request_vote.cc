#include <string.h>
#include <omnetpp.h>
#include <string>
using namespace omnetpp;
using namespace std;

class RequestVote: public cMessage {
private:
    int term;
    int candidateId;
    int lastLogIndex;
    int lastLogTerm;
public:
    RequestVote(string name,int term,int candidateId,int lastLogIndex, int lastLogTerm)

    {
      this->setName(name.c_str());
      this->term=term;
      this->candidateId=candidateId;
      this->lastLogIndex=lastLogIndex;
      this->lastLogTerm=lastLogTerm;
    }

    int getCandidateId() const {
        return candidateId;
    }

    void setCandidateId(int candidateId) {
        this->candidateId = candidateId;
    }

    int getLastLogIndex() const {
        return lastLogIndex;
    }

    void setLastLogIndex(int lastLogIndex) {
        this->lastLogIndex = lastLogIndex;
    }

    int getLastLogTerm() const {
        return lastLogTerm;
    }

    void setLastLogTerm(int lastLogTerm) {
        this->lastLogTerm = lastLogTerm;
    }

    int getTerm() const {
        return term;
    }

    void setTerm(int term) {
        this->term = term;
    }

    cMessage* dup() const override {

        return new RequestVote("RequestVote",this->term,this->candidateId,this->lastLogIndex,this->lastLogTerm);
    }
};
