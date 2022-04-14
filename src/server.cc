#include "server.h"
#include <typeinfo>
#include <list>
#include <string>
#include "messages/rpc/append_entries/append_entries.h"
#include "messages/rpc/append_entries/append_entries_response.h"
#include "messages/rpc/request_vote/request_vote.h"
#include "messages/rpc/request_vote/request_vote_response.h"

void Server::initialize() {
    nextIndex = new int(getVectorSize());
    matchIndex = new int(getVectorSize());
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    resendAppendEntryEvent = new cMessage("retryAppendEntryEvent");
    heartbeatEvent = new cMessage("heartbeatEvent");
    faultywhenleader = par("faultyWhenLeader");
    rescheduleElectionTimeout();
    WATCH(currentTerm);
    WATCH(currentState);
    WATCH(votedFor);
    WATCH(electionTimeout);
}

void Server::finish() {
    cancelEvent(electionTimeoutEvent);
    cancelEvent(resendAppendEntryEvent);
    cancelEvent(heartbeatEvent);
    EV << "[Server" << getIndex() << "] Votes count is " << votesCount << endl;
}

void Server::handleMessage(cMessage *msg) {
    if (crashed) {
        delete msg;
        return;
    }

    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {
        if (msg == heartbeatEvent) {
            std::list<LogEntry> empty_log = { };
            AppendEntries *heartbeat = new AppendEntries("Heartbeat",
                    currentTerm, getIndex(), getLastLogIndex(),
                    getLastLogTerm(), empty_log, commitIndex);
            broadcast(heartbeat);

            // Test what happens if a leader do not send HeartBeats to other servers anymore
            if (faultywhenleader && uniform(0, 1) > 0.6) {
                bubble("definitely crashed");
                crashed = true;
                return;
            }

            scheduleHeartbeat();
            return;
        }

        if (msg == electionTimeoutEvent) {
            startElection();
            return;
        }

        // Re-send to all servers the log they need
        // in order to be consistent with the leader
        if (msg == resendAppendEntryEvent) {
            bool allServersConsistent = true;
            for (int serverIndex = 0; serverIndex < gateSize("out");
                    serverIndex++) {
                if (log.size() >= nextIndex[serverIndex]) {
                    allServersConsistent = false;
                    list<LogEntry>::iterator it = log.begin();
                    advance(it, nextIndex[serverIndex] - 1);
                    list<LogEntry> tosend = { *it };
                    send(
                            new AppendEntries("AppendEntries", currentTerm,
                                    getIndex(), getLastLogIndex(),
                                    getLastLogTerm(), tosend, commitIndex),
                            "out", serverIndex);
                }
            }
            if (!allServersConsistent) { //retry appendentries until all servers are consistent with the log of the leader

                simtime_t appendEntryPeriod = par("retryAppendEntriesPeriod");
                scheduleAt(simTime() + appendEntryPeriod,
                        resendAppendEntryEvent);
            }
            return;
        }

        return;
    }

    // *** EXTERNAL MESSAGES ***
    // All messages from this point on are sent from other servers/clients

    EV << "[Server" << getIndex() << "] Message received from Server"
              << msg->getSenderModule()->getIndex() << " ~ " << msg->getName()
              << endl;

    // Generic behavior for RPC messages
    if (dynamic_cast<RPC*>(msg) != nullptr) {
        RPC *rpc = check_and_cast<RPC*>(msg);

        // "If RPC request or response contains term T > currentTerm:
        // set currentTerm = T, convert to follower"
        if (rpc->term > currentTerm) {
            currentTerm = rpc->term;
            votedFor = -1;
            votesCount = 0;

            if (currentState == LEADER)
                cancelHeartbeat();
            currentState = FOLLOWER;
        }
    }

    // Generic behavior for RPCRequest messages
    if (dynamic_cast<RPCRequest*>(msg) != nullptr) {
        RPCRequest *rpc = check_and_cast<RPCRequest*>(msg);

        // "If a server receives a request with a stale term
        // number, it rejects the request"
        if (rpc->term < currentTerm) {
            rpc->buildAndSendResponse(this, false);
        }
    }

    HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
    handableMsg->handleOnServer(this);
}

void Server::startElection() {
    electionTimeout = par("electionTimeout");
    scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
    currentTerm = currentTerm + 1;
    votesCount = 0; // Reset votes count from previous election
    EV << "[Server" << getIndex() << "] Start election at " << simTime()
              << " , term = " << currentTerm << endl;

    currentState = CANDIDATE;
    votesCount++;
    votedFor = getIndex();
    RequestVote *requestvote = new RequestVote("RequestVote", currentTerm,
            getIndex(), getLastLogIndex(), getLastLogTerm());

// cMessage *mextobroadcast = &requestvote;
    broadcast(requestvote);
}

void Server::scheduleHeartbeat() {
    cancelEvent(heartbeatEvent);
    simtime_t heartbeatPeriod = par("heartbeatPeriod");
    scheduleAt(simTime() + heartbeatPeriod, heartbeatEvent);
}

void Server::cancelHeartbeat() {
    cancelEvent(heartbeatEvent);
}

void Server::rescheduleElectionTimeout() {
    cancelEvent(electionTimeoutEvent);

    simtime_t electionTimeout = par("electionTimeout");
    scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
}

void Server::stopElectionTimeout() {
    cancelEvent(electionTimeoutEvent);
}

void Server::scheduleResendAppendEntries() {
    simtime_t appendEntryPeriod = par("retryAppendEntriesPeriod");
    scheduleAt(simTime() + appendEntryPeriod, resendAppendEntryEvent);
}
void Server::cancelResendAppendEntries() {
    cancelEvent(resendAppendEntryEvent);
}

void Server::broadcast(cMessage *msg) {
    for (int i = 0; i < gateSize("out"); i++)
        send(msg->dup(), "out", i);
}

int Server::getLastLogIndex() {
    int lastlogindex = log.size();
    return lastlogindex;
}

int Server::getLastLogTerm() {
    int lastlogterm;

    if (log.empty())
        lastlogterm = 0;
    else {
        LogEntry lastentry = log.front();
        lastlogterm = lastentry.getLogTerm();
    }

    return lastlogterm;
}

