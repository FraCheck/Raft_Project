#ifndef MESSAGES_HANDABLE_MESSAGE_H_
#define MESSAGES_HANDABLE_MESSAGE_H_

#include "omnetpp.h"
#include "../server.h"
#include "../client.h"

using namespace omnetpp;

class HandableMessage: public cMessage {
public:
    virtual void handleOnServer(Server *server) const {
    }
    ;
    virtual void handleOnClient(Client *client) const {
    }
    ;

    void finish() {
        delete this;
    }
};

#endif
