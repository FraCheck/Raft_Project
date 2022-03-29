#ifndef MESSAGES_REQUEST_VOTE_REQUEST_VOTE_RESPONSE_H_
#define MESSAGES_REQUEST_VOTE_REQUEST_VOTE_RESPONSE_H_

#include "../handable_message.h"

class RequestVoteResponse: public HandableMessage {
private:
    int term;
    bool voteGranted;

public:
    RequestVoteResponse(int term, bool voteGranted);

    void handleOnServer(Server* server) const override;
    cMessage* dup() const override;

    // Getter methods
    int getTerm() const;
    bool getVoteGranted() const;
};

#endif
