#ifndef MESSAGES_ADD_COMMAND_RESPONSE_H_
#define MESSAGES_ADD_COMMAND_RESPONSE_H_

#include "../handable_message.h"

class AddCommandResponse: public HandableMessage {
public:
    bool success;
    int leaderId;
    int requestId;
    AddCommandResponse(bool success, int leaderId, int requestId);
    void handleOnClient(Client *client) const override;
    cMessage* dup() const override;
};

#endif
