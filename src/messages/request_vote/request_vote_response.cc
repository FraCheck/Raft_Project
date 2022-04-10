#include "request_vote_response.h"

RequestVoteResponse::RequestVoteResponse(int term, bool voteGranted) {
    cMessage::setName("RequestVoteResponse");
    this->term = term;
    this->voteGranted = voteGranted;
}

void RequestVoteResponse::handleOnServer(Server *server) const {

    if (!voteGranted) {
        if(term>server->currentTerm){
                       server->currentTerm=term;
                       server->currentState=FOLLOWER;
                       server->votesCount=0;
                   }
        server->rescheduleElectionTimeout();
        return;
    }

    server->votesCount++;
    server->cancelEvent(server->electionTimeoutEvent);

    if (server->votesCount > server->getVectorSize() / 2) {
        server->currentState = LEADER;
        server->currentLeader = server->getIndex();
        for(int i =0;i<server->getVectorSize();i++){
            server->nextIndex[i]=1;
            server->matchIndex[i]=0;
        }

        server->scheduleHeartbeat();
    } else
        server->rescheduleElectionTimeout();

    return;
}

cMessage* RequestVoteResponse::dup() const {
    return new RequestVoteResponse(term, voteGranted);
}

int RequestVoteResponse::getTerm() const {
    return term;
}

bool RequestVoteResponse::getVoteGranted() const {
    return voteGranted;
}

