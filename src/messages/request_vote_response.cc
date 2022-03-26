#include <omnetpp.h>

using namespace omnetpp;
using namespace std;

class RequestVoteResponse: public cMessage {
private:
    int term;
    bool voteGranted;
public:
    RequestVoteResponse(int term, bool voteGranted) {
        this->setName("RequestVoteResponse");
        this->term = term;
        this->voteGranted = voteGranted;
    }

    int getTerm() {
        return term;
    }

    bool getVoteGranted() {
        return voteGranted;
    }

    cMessage* dup() const override {
        return new RequestVoteResponse(term, voteGranted);
    }
};
