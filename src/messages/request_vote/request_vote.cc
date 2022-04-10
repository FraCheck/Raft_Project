#include "request_vote.h"
#include "request_vote_response.h"
RequestVote::RequestVote(string name, int term, int candidateId,
        int lastLogIndex, int lastLogTerm) {
    cMessage::setName(name.c_str());
    this->term = term;
    this->candidateId = candidateId;
    this->lastLogIndex = lastLogIndex;
    this->lastLogTerm = lastLogTerm;
}

void RequestVote::handleOnServer(Server *server) const {

    server->cancelEvent(server->electionTimeoutEvent);

    // Each server will vote for at most one candidate in a given term,
    // on a first come-first-served-basis
    if (term < server->currentTerm ) {
        server->send(new RequestVoteResponse(server->currentTerm, false), "out",
                getArrivalGate()->getIndex());
        if(!(server->currentState==LEADER))server->rescheduleElectionTimeout();
        return;
    }

    if (term > server->currentTerm || (term == server->currentTerm && (server->votedFor==-1 || server->votedFor==this->candidateId) ) ) {
        // Deny the vote if candidate log is not up to date with the current one
        if (term > server->getLastLogTerm()
                || (lastLogTerm == server->getLastLogTerm()
                        && lastLogIndex >= server->getLastLogIndex())) {
            server->send(new RequestVoteResponse(server->currentTerm, true),
                    "out", getArrivalGate()->getIndex());

            server->votedFor = candidateId;

        } else
            server->send(new RequestVoteResponse(server->currentTerm, false),
                    "out", getArrivalGate()->getIndex());
    }
    else {
        server->send(new RequestVoteResponse(server->currentTerm, false),
                            "out", getArrivalGate()->getIndex());
    }
    if(term>server->currentTerm){
                   server->currentTerm=term;
                   server->currentState=FOLLOWER;
                   server->votesCount=0;
               }
    if(!(server->currentState==LEADER))server->rescheduleElectionTimeout();
}

cMessage* RequestVote::dup() const {
    return new RequestVote(getName(), term, candidateId, lastLogIndex,
            lastLogTerm);
}

int RequestVote::getTerm() const {
    return term;
}

int RequestVote::getCandidateId() const {
    return candidateId;
}

int RequestVote::getLastLogIndex() const {
    return lastLogIndex;
}

int RequestVote::getLastLogTerm() const {
    return lastLogTerm;
}

