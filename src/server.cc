#include <string.h>
#include <omnetpp.h>
#include <typeinfo>
#include "messages/request_vote.cc"
#include "utils/log_entry.h"

#include <list>
#include <string>

using namespace omnetpp;
using namespace std;

enum ServerState {
    Leader, Follower, Candidate
};

class Server: public cSimpleModule {
    ServerState currentState = Follower;

    cMessage *electionTimeoutEvent;    //  message for election timeout
    cMessage *heartbeatEvent;
    int votesCount = 0;
    bool hasVoted = false;
    bool faultywhenleader;
    bool crashed = false;

    int currentTerm = 0;    //persistent state on all servers
    int votedFor = -1;
    list<LogEntry> log = { };

    int commitIndex = 0;     //volatile state on all servers
    int lastApplied = 0;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
private:
    void startElection();
    void scheduleHeartbeat();

    void broadcast(cMessage *msg);
    int getLastLogTerm();
    int getLastLogIndex();
};

Define_Module (Server);

void Server::initialize() {
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    faultywhenleader = par("faultywhenleader");
    simtime_t electionTimeout = par("electionTimeout");
    scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
    WATCH(currentTerm);
    WATCH(votedFor);
}

void Server::finish() {
    cancelEvent(electionTimeoutEvent);

    EV << "[Server" << getIndex() << "] Votes count is " << votesCount << endl;
}

void Server::handleMessage(cMessage *msg) {
    if (crashed) {
        delete msg;
        return;
    }
    if (msg == heartbeatEvent) {
        broadcast(new cMessage("Heartbeat"));

        if (faultywhenleader && uniform(0, 1) > 0.7) { // this is useful to test what happens if a leader do not send HeartBeats to other servers anymore
            bubble("definitely crashed");
            crashed = true;
            return;
        }
        scheduleHeartbeat();
    }

    if (msg->isName("Heartbeat")) {    //to implement with appendentries message
        cancelEvent(electionTimeoutEvent);
        simtime_t electionTimeout = par("electionTimeout");
        scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
    }

    if (msg == electionTimeoutEvent) {
        startElection();
        return;
    }

    EV << "[Server" << getIndex() << "] Message received from Server"
            << msg->getSenderModule()->getIndex() << " ~ " << msg->getName()
            << endl;

    if (msg->isName("RequestVote")) {
        cancelEvent(electionTimeoutEvent);
        RequestVote *requestvote = check_and_cast<RequestVote*>(msg);
        // Each server will vote for at most one candidate in a given term, on a first come-first-served-basis
        if (requestvote->getTerm() > currentTerm) {

            //denying the vote if candidate log is less up to date than voter log
            if (requestvote->getLastLogTerm() > getLastLogTerm()
                    || (requestvote->getLastLogTerm() == getLastLogTerm()
                            && requestvote->getLastLogIndex()
                                    >= getLastLogIndex())) {
                send(new cMessage("Vote"), "out",
                        msg->getArrivalGate()->getIndex());
                votedFor = requestvote->getCandidateId();
                currentTerm = requestvote->getTerm();
            }
        }
        simtime_t electionTimeout = par("electionTimeout");
        scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
    }

    if (msg->isName("Vote")) {
        votesCount++;
        cancelEvent(electionTimeoutEvent);
        if (votesCount > getVectorSize() / 2) {
            currentState = Leader;

            scheduleHeartbeat();
        } else {
            simtime_t electionTimeout = par("electionTimeout");
            scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
        }
        return;
    }
}

void Server::startElection() {
    currentTerm = currentTerm + 1;

    EV << "[Server" << getIndex() << "] Start election at " << simTime()
            << " , term = " << currentTerm << endl;

    currentState = Candidate;
    votesCount++;
    RequestVote requestvote = RequestVote("RequestVote", currentTerm,
            getIndex(), getLastLogIndex(), getLastLogTerm());
    cMessage *mextobroadcast = &requestvote;
    broadcast(mextobroadcast);
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
int Server::getLastLogIndex() {
    int lastlogindex;

    if (log.empty())
        lastlogindex = 0;
    else {
        LogEntry lastentry = log.front();
        lastlogindex = log.size();
    }
    return lastlogindex;
}
int Server::getLastLogTerm() {
    int lastlogterm;
    if (log.empty())
        lastlogterm = 0;
    else {
        LogEntry lastentry = log.front();
        lastlogterm = lastentry.getLogterm();
    }
    return lastlogterm;
}
