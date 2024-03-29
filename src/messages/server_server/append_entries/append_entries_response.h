#ifndef MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_RESPONSE_H_
#define MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_RESPONSE_H_

#include "../../handable_message.h"
#include "../rpc.h"

class AppendEntriesResponse: public HandableMessage, public RPCResponse {
public:
    // Index of the last LogEntry added in the follower log
    int lastLogIndex;
    bool successterm;

    AppendEntriesResponse(int term, bool successterm,bool successconsistency, int lastLogIndex) :
            RPCResponse(term, successconsistency) {
        this->lastLogIndex = lastLogIndex;
        this->successterm= successterm;
    }

    void handleOnServer(Server *server) const override;
    void buildAndSendNextAppendEntriesRequest(Server *server,
            int senderIndex) const;

    const char* getDisplayString() const override {
        return RPCResponse::result ?
                "b=14,14,oval,green,black,1.2" : "b=14,14,oval,red,black,1.2";
    }

    cMessage* dup() const override {
        return new AppendEntriesResponse(term, successterm,result, lastLogIndex);
    }
};

#endif
