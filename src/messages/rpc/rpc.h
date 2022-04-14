#ifndef MESSAGES_RPC_RPC_H_
#define MESSAGES_RPC_RPC_H_

#include "../../server.h"

class RPC {
public:
    int term;
    RPC(int term) {
        this->term = term;
    }
};

class RPCRequest: public RPC {
public:
    RPCRequest(int term) :
            RPC(term) {
    }

    virtual void buildAndSendResponse(Server *server, bool success) const = 0;
};

class RPCResponse: public RPC {
public:
    bool result;

    RPCResponse(int term, bool result) :
            RPC(term) {
        this->result = result;
    }
};

#endif
