#include "server.h"
#include <typeinfo>
#include <list>
#include <string>

#include "messages/server_server/append_entries/append_entries.h"
#include "messages/server_server/append_entries/append_entries_response.h"
#include "messages/server_server/request_vote/request_vote.h"
#include "messages/server_server/request_vote/request_vote_response.h"
#include "messages/server_statsCollector/leader_elected.h"
#include "messages/server_statsCollector/server_failure.h"
#include "utils/printer.h"

void Server::refreshDisplay() const {
    ostringstream out;
    cDisplayString &dispStr = getDisplayString();

    if (state == FOLLOWER)
        out << "i=block/circle;";
    if (state == LEADER)
        out << "i=block/triangle;";
    if (state == CANDIDATE)
        out << "i=block/square;";

    if (crashed)
        out << "i2=status/stop;";

    if (state == LEADER)
        out << "t=nextIndex: " << printElements(nextIndex, getVectorSize())
                << endl << "matchIndex: "
                << printElements(matchIndex, getVectorSize());

    dispStr.parse(out.str().c_str());

    ostringstream labelText;
    labelText << padOut(to_string(getIndex()), 3)
            << padOut(to_string(currentTerm), 6)
            << padOut(to_string(votedFor), 6)
            << padOut(to_string(commitIndex), 8)
            << padOut(to_string(lastApplied), 7) << "[" << log->toString()
            << "]";

    label->setText(labelText.str().c_str());
    label->setPosition(cFigure::Point(20, 6 + getIndex() * 6));
    label->setFont(cFigure::Font("Courier New"));
    label->setAnchor(cFigure::ANCHOR_NW);

    cCanvas *canvas = getParentModule()->getCanvas();
    if (canvas->findFigure(label) != -1)
        canvas->removeFigure(label);
    canvas->addFigure(label);
}

void Server::initialize() {
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    resendAppendEntryEvent = new cMessage("retryAppendEntryEvent");
    heartbeatEvent = new cMessage("heartbeatEvent");

    rescheduleElectionTimeout();

    canFail = par("canFail");
    if (canFail) {
        crashEvent = new cMessage("crashEvent");
        recoverEvent = new cMessage("recoverEvent");

        scheduleCrash();
    }

    WATCH(currentTerm);
    WATCH(state);
    WATCH(votedFor);
    WATCH(electionTimeout);

    cLabelFigure *title = new cLabelFigure();
    title->setText("#  term  vote  commit  apply  logEntries (term-index)");
    title->setPosition(cFigure::Point(20, 0));
    title->setFont(cFigure::Font("Courier New"));
    title->setAnchor(cFigure::ANCHOR_NW);

    cCanvas *canvas = getParentModule()->getCanvas();
    canvas->addFigure(title);
}

void Server::finish() {
    cancelEvent(electionTimeoutEvent);
    cancelEvent(resendAppendEntryEvent);
    cancelEvent(heartbeatEvent);
}

void Server::handleMessage(cMessage *msg) {
    if (msg == crashEvent) {
        bubble("CRASHED");
        crashed = true;
        scheduleRecover();

        ServerFailure *failed = new ServerFailure();
        send(failed, "toStatsCollector");

        return;
    }

    if (msg == recoverEvent) {
        bubble("RECOVERED");
        crashed = false;
        scheduleCrash();

        return;
    }

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

        // If commitIndex > lastApplied: increment lastApplied, apply
        // log[lastApplied] to state machine
        if (commitIndex > lastApplied)
            lastApplied++;

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

void Server::registerLeaderElectionTime() {
    LeaderElected *elected = new LeaderElected();
    send(elected, "toStatsCollector");
}

void Server::scheduleHeartbeat() {
    cancelEvent(heartbeatEvent);
    simtime_t heartbeatPeriod = par("heartbeatPeriod");
    // EV << "SIMULATION TIMES TEST, heartbeatPeriod: "<<heartbeatPeriod<< "   || simTime():  " << simTime() << "   ||SUM: "<< simTime() + heartbeatPeriod ;

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

void Server::scheduleCrash() {
    simtime_t crashTimeout = par("crashTimeout");
    scheduleAt(simTime() + crashTimeout, crashEvent);
}

void Server::scheduleRecover() {
    simtime_t recoverTimeout = par("recoverTimeout");
    scheduleAt(simTime() + recoverTimeout, recoverEvent);
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
