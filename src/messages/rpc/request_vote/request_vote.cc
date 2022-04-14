#include "../../rpc/request_vote/request_vote.h"
#include "../../rpc/request_vote/request_vote_response.h"

void RequestVote::handleOnServer(Server *server) const {
    server->stopElectionTimeout();
    if (server->currentState != LEADER)
        server->rescheduleElectionTimeout();

    // "Each server will vote for at most one candidate in a given term,
    // on a first come-first-served-basis"

    // Reject requests if the server has already voted
    if (server->votedFor != -1 && server->votedFor != this->candidateId) {
        buildAndSendResponse(server, false);
        return;
    }

    // Reject requests if the candidate log is not up to date
    if (term <= server->getLastLogTerm()
            && (lastLogTerm != server->getLastLogTerm()
                    || lastLogIndex < server->getLastLogIndex())) {
        buildAndSendResponse(server, false);
        return;
    }

    buildAndSendResponse(server, true);
    server->votedFor = candidateId;
}

void RequestVote::buildAndSendResponse(Server *server, bool success) const {
    RequestVoteResponse *response = new RequestVoteResponse(server->currentTerm,
            success);
    server->send(response, "out", getArrivalGate()->getIndex());
}
