#include <omnetpp.h>

using namespace omnetpp;
using namespace std;

class AppendEntriesResponse: public cMessage {
private:
    int term;
    bool success;
public:
    AppendEntriesResponse(int term, bool success) {
        this->setName("AppendEntriesResponse");
        this->term = term;
        this->success = success;
    }

    int getTerm() {
        return term;
    }

    bool getSuccess() {
        return success;
    }

    cMessage* dup() const override {
        return new AppendEntriesResponse(term, success);
    }
};
