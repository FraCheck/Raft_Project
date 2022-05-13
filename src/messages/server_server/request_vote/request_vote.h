#ifndef MESSAGES_REQUEST_VOTE_REQUEST_VOTE_H_
#define MESSAGES_REQUEST_VOTE_REQUEST_VOTE_H_

#include "../../handable_message.h"
#include "../../server_server/rpc.h"

using namespace omnetpp;
using namespace std;

class RequestVote: public HandableMessage, public RPCRequest {
private:
    // Candidate requesting vote
    int candidateId;

    // Index of candidate’s last log entry
    int lastLogIndex;

    // Term of candidate’s last log entry
    int lastLogTerm;

public:
    RequestVote(string name, int term, int candidateId, int lastLogIndex,
            int lastLogTerm) :
            RPCRequest(term) {
        cMessage::setName(name.c_str());
        this->candidateId = candidateId;
        this->lastLogIndex = lastLogIndex;
        this->lastLogTerm = lastLogTerm;
    }

    void handleOnServer(Server *server) const override;
    void buildAndSendResponse(Server *server, bool success) const override;

    const char* getDisplayString() const override {
        return "b=14,14,oval,yellow,black,1.2";;
    }

    cMessage* dup() const override {
        return new RequestVote(getName(), term, candidateId, lastLogIndex,
                lastLogTerm);
    }
};

#endif
