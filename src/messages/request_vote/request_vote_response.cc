#include "request_vote_response.h"

RequestVoteResponse::RequestVoteResponse(int term, bool voteGranted) {
    cMessage::setName("RequestVoteResponse");
    this->term = term;
    this->voteGranted = voteGranted;
}

void RequestVoteResponse::handleOnServer(Server *server) const {

    if (!voteGranted) {
        // TODO: implement the correct behavior
        return;
    }

    server->votesCount++;
    server->cancelEvent(electionTimeoutEvent);

    if (server->votesCount > vectorSize / 2) {
        server->currentState = LEADER;

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

