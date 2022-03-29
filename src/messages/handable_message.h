#ifndef MESSAGES_HANDABLE_MESSAGE_H_
#define MESSAGES_HANDABLE_MESSAGE_H_

#include "omnetpp.h"
#include "../server.h"

using namespace omnetpp;

class Server;
class HandableMessage: public cMessage {
public:
    virtual void handleOnServer(Server *server) const = 0;
};

#endif
