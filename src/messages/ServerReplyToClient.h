
#ifndef MESSAGES_SERVERREPLYTOCLIENT_H_
#define MESSAGES_SERVERREPLYTOCLIENT_H_
#include "handable_message.h"
class ServerReplyToClient : public HandableMessage {
public:
    bool success;
    int leaderId;
    int requestId;
    ServerReplyToClient(bool success, int leaderId,int requestId);
    void handleOnClient(Client* client) const override;
        cMessage* dup() const override;
};

#endif /* MESSAGES_SERVERREPLYTOCLIENT_H_ */
