#include <omnetpp.h>

using namespace omnetpp;
using namespace std;

class RequestVoteResponse: public cMessage {
private:
    int term;
    bool voteGranted;
public:
    int getTerm() {
        return term;
    }
    bool getVoteGranted() {
        return voteGranted;
    }

    cMessage* dup() const override {
        return new RequestVoteResponse();
    }
};
