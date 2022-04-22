#include "server.h"
#include <typeinfo>
#include <list>
#include <string>

#include "messages/server_server/append_entries/append_entries.h"
#include "messages/server_server/append_entries/append_entries_response.h"
#include "messages/server_server/request_vote/request_vote.h"
#include "messages/server_server/request_vote/request_vote_response.h"
#include "utils/printer.h"

void Server::initialize() {
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    resendAppendEntryEvent = new cMessage("retryAppendEntryEvent");
    heartbeatEvent = new cMessage("heartbeatEvent");

    faultywhenleader = par("faultyWhenLeader");

    rescheduleElectionTimeout();

    WATCH(currentTerm);
    WATCH(state);
    WATCH(votedFor);
    WATCH(electionTimeout);
}

void Server::finish() {
    cancelEvent(electionTimeoutEvent);
    cancelEvent(resendAppendEntryEvent);
    cancelEvent(heartbeatEvent);
}

void Server::handleMessage(cMessage *msg) {
    if (crashed) {
        delete msg;
        return;
    }

    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {
        if (msg == heartbeatEvent) {
            AppendEntries *heartbeat = new AppendEntries("Heartbeat",
                    currentTerm, getIndex(), getLastLogIndex(),
                    getLastLogTerm(), { }, commitIndex);
            broadcast(heartbeat);

            // Test what happens if a leader do not send HeartBeats anymore
            if (faultywhenleader && uniform(0, 1) > 0.6) {
                bubble("definitely crashed");
                crashed = true;
                cDisplayString &dispStr = getDisplayString();
                dispStr.parse("i=block/process");
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
            for (int i = 0; i < gateSize("out"); i++) {
                if (log->size() >= nextIndex[i]) {
                    allServersConsistent = false;

                    AppendEntries *request = new AppendEntries("AppendEntries",
                            currentTerm, getIndex(), getLastLogIndex(),
                            getLastLogTerm(),
                            { log->getFromIndex(nextIndex[i]) }, commitIndex);
                    send(request, "out", i);
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

    // Generic behavior for RPC messages
    if (dynamic_cast<RPC*>(msg) != nullptr) {
        RPC *rpc = check_and_cast<RPC*>(msg);

        // "If RPC request or response contains term T > currentTerm:
        // set currentTerm = T, convert to follower"
        if (rpc->term > currentTerm) {
            currentTerm = rpc->term;
            votedFor = -1;
            votesCount = 0;

            if (state == LEADER)
                cancelHeartbeat();
            state = FOLLOWER;
        }
    }

    // Generic behavior for RPCRequest messages
    if (dynamic_cast<RPCRequest*>(msg) != nullptr) {
        RPCRequest *rpc = check_and_cast<RPCRequest*>(msg);

        stopElectionTimeout();
        if (state != LEADER)
            rescheduleElectionTimeout();

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

    state = CANDIDATE;
    votesCount++;
    votedFor = getIndex();

    RequestVote *requestvote = new RequestVote("RequestVote", currentTerm,
            getIndex(), getLastLogIndex(), getLastLogTerm());
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
    cancelEvent(resendAppendEntryEvent);

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

int Server::getLastLogTerm() {
    if (log->size() == 0)
        return 0;

    return log->getLast().term;

}

int Server::getLastLogIndex() {
    if (log->size() == 0)
        return 0;

    return log->getLast().index;
}

void Server::logNextAndMatchIndexes() {
    EV << "nextIndex: " << printElements(nextIndex, getVectorSize()) << endl;
    EV << "matchIndex: " << printElements(matchIndex, getVectorSize()) << endl;
}

