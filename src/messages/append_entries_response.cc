using namespace omnetpp;
using namespace std;

class AppendEntriesResponse: public cMessage {
private:
    int term;
    bool success;
public:
    int getTerm() {
        return term;
    }

    bool getSuccess() {
        return success;
    }

    cMessage* dup() const override {
        return new AppendEntriesResponse();
    }
};
