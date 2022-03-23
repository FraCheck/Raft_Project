#include <string.h>
#include <omnetpp.h>
#include <typeinfo>
#include "messages/request_vote.cc"
#include "messages/append_entries.cc"
#include "messages/request_vote_response.cc"
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
    cMessage *heartbeatEvent;          // message for heartbeat timeout

    int votesCount = 0;
    bool faultywhenleader;
    bool crashed = false;

    // Persistent state on all servers
    int currentTerm = 0;        // Latest term server has seen (initialized to 0 on first boot, increases monotonically)
    int votedFor = -1;          // CandidateId that received vote in current term (or null if none)
    list<LogEntry> log = { };   // Log entries

    // Volatile state on all servers
    int commitIndex = 0;        // Index of highest log entry known to be committed (initialized to 0, increases monotonically)
    int lastApplied = 0;        // Index of highest log entry applied to state machine (initialized to 0, increases monotonically)

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

Define_Module(Server);

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
        std::list<LogEntry> empty_log = { };

        AppendEntries appendentries = AppendEntries("Heartbeat", currentTerm, getIndex(), getLastLogIndex(), getLastLogTerm(), empty_log, commitIndex) ;
        cMessage *mextobroadcast = &appendentries;
        broadcast(mextobroadcast);
        if (faultywhenleader && uniform(0, 1) > 0.7) { // this is useful to test what happens if a leader do not send HeartBeats to other servers anymore
            bubble("definitely crashed");
            crashed = true;
            return;
        }
        scheduleHeartbeat();
    }


    if (msg->isName("Heartbeat")) {
        AppendEntries *appendentries = check_and_cast<AppendEntries*>(msg);

        if ((appendentries->getEntries()).empty()){
            cancelEvent(electionTimeoutEvent);
            simtime_t electionTimeout = par("electionTimeout");
            scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
        }else
            throw "Protocol Violation: received a Heartbeat message with non-null log_entries list.";
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

        // Each server will vote for at most one candidate in a given term,
        // on a first come-first-served-basis
        if (votedFor != -1)
            return;

        RequestVote *requestVote = check_and_cast<RequestVote*>(msg);
        if (requestVote->getTerm() > currentTerm) {

            // Deny the vote if candidate log is not up to date with the current one
            if (requestVote->getLastLogTerm() > getLastLogTerm()
                    || (requestVote->getLastLogTerm() == getLastLogTerm()
                            && requestVote->getLastLogIndex()
                                    >= getLastLogIndex())) {
                send(new RequestVoteResponse(currentTerm, true), "out",
                        msg->getArrivalGate()->getIndex());

                votedFor = requestVote->getCandidateId();
                currentTerm = requestVote->getTerm();
            } else
                send(new RequestVoteResponse(currentTerm, false), "out",
                        msg->getArrivalGate()->getIndex());
        }

        simtime_t electionTimeout = par("electionTimeout");
        scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
    }

    if (msg->isName("RequestVoteResponse")) {
        RequestVoteResponse *response = check_and_cast<RequestVoteResponse*>(
                msg);

        if (!response->getVoteGranted())
            // TODO: implement the correct behavior
            return;

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
    votedFor = getIndex();
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
