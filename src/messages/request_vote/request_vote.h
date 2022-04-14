#ifndef MESSAGES_REQUEST_VOTE_REQUEST_VOTE_H_
#define MESSAGES_REQUEST_VOTE_REQUEST_VOTE_H_

#include "../handable_message.h"

using namespace omnetpp;
using namespace std;

class RequestVote: public HandableMessage {
private:
    int term;
    int candidateId;
    int lastLogIndex;
    int lastLogTerm;
public:
    RequestVote(string name, int term, int candidateId, int lastLogIndex,
            int lastLogTerm);

    void handleOnServer(Server* server) const override;
    cMessage* dup() const override;

    // Getter methods
    int getTerm() const;
    int getCandidateId() const;
    int getLastLogIndex() const;
    int getLastLogTerm() const;
};

#endif
