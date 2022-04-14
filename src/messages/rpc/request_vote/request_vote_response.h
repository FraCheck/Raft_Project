#ifndef MESSAGES_REQUEST_VOTE_REQUEST_VOTE_RESPONSE_H_
#define MESSAGES_REQUEST_VOTE_REQUEST_VOTE_RESPONSE_H_

#include "../../handable_message.h"
#include "../rpc.h"

class RequestVoteResponse: public HandableMessage, public RPCResponse {
public:
    RequestVoteResponse(int term, bool voteGranted) :
            RPCResponse(term, voteGranted) {
        cMessage::setName(voteGranted ? "TRUE" : "FALSE");
    }

    void handleOnServer(Server *server) const override;

    cMessage* dup() const override {
        return new RequestVoteResponse(term, result);
    }
};

#endif
