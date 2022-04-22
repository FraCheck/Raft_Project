#ifndef MESSAGES_REQUEST_VOTE_REQUEST_VOTE_RESPONSE_H_
#define MESSAGES_REQUEST_VOTE_REQUEST_VOTE_RESPONSE_H_

#include "../../handable_message.h"
#include "../../server_server/rpc.h"

class RequestVoteResponse: public HandableMessage, public RPCResponse {
public:
    RequestVoteResponse(int term, bool voteGranted) :
            RPCResponse(term, voteGranted) {
    }

    void handleOnServer(Server *server) const override;

    const char* getDisplayString() const override {
        return RPCResponse::result ?
                "b=14,14,oval,green,black,1.2" : "b=14,14,oval,red,black,1.2";
    }

    cMessage* dup() const override {
        return new RequestVoteResponse(term, result);
    }
};

#endif
