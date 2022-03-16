#include <string.h>
#include <omnetpp.h>
#include <typeinfo>
#include "messages/request_vote.cc"

using namespace omnetpp;

enum ServerState {
    Leader, Follower, Candidate
};

class Server: public cSimpleModule {
    ServerState currentState = Follower;

    cMessage *electionTimeoutEvent;
    cMessage *heartbeatEvent;
    int votesCount = 0;
    bool hasVoted = false;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
private:
    void startElection();
    void scheduleHeartbeat();

    void broadcast(cMessage *msg);
};

Define_Module(Server);

void Server::initialize() {
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");

    simtime_t electionTimeout = par("electionTimeout");
    scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
}

void Server::finish() {
    cancelEvent(electionTimeoutEvent);

    EV << "[Server" << getIndex() << "] Votes count is " << votesCount << endl;
}

void Server::handleMessage(cMessage *msg) {
    if (msg == heartbeatEvent) {
        broadcast(new cMessage("Heartbeat"));
        scheduleHeartbeat();
    }

    if (msg == electionTimeoutEvent) {
        startElection();
        return;
    }

    cancelEvent(electionTimeoutEvent);

    EV << "[Server" << getIndex() << "] Message received from Server"
              << msg->getSenderModule()->getIndex() << " ~ " << msg->getName()
              << endl;

    if (msg->isName("RequestVote") && !hasVoted) {
        // Each server will vote for at most one candidate in a given term, on a first come-first-served-basis
        send(new cMessage("Vote"), "out", msg->getArrivalGate()->getIndex());
        hasVoted = true;
    }

    if (msg->isName("Vote")) {
        votesCount++;
        if (votesCount > getVectorSize() / 2) {
            currentState = Leader;
            scheduleHeartbeat();
        }
        return;
    }
}

void Server::startElection() {
    EV << "[Server" << getIndex() << "] Start election at " << simTime()
              << endl;

    currentState = Candidate;
    votesCount++;
    hasVoted = true;

    broadcast(new cMessage("RequestVote"));
}

void Server::scheduleHeartbeat() {
    heartbeatEvent = new cMessage("heartbeatEvent");

    simtime_t heartbeatPeriod = par("heartbeatPeriod");
    scheduleAt(simTime() + heartbeatPeriod, heartbeatEvent);
}

void Server::broadcast(cMessage *msg) {
    for (int i = 0; i < gateSize("out"); i++) {
        send(msg->dup(), "out", i);
    }
}

