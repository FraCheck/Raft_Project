#ifndef MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_RESPONSE_H_
#define MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_RESPONSE_H_

#include "../../handable_message.h"
#include "../rpc.h"

class AppendEntriesResponse: public HandableMessage, public RPCResponse {
private:
    int index;

public:
    AppendEntriesResponse(int term, bool success, int index) :
            RPCResponse(term, success) {
        cMessage::setName(success ? "TRUE" : "FALSE");
        this->index = index;
    }

    void handleOnServer(Server *server) const override;

    cMessage* dup() const override {
        return new AppendEntriesResponse(term, result, index);
    }
};

#endif
