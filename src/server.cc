#include "server.h"
#include <typeinfo>
#include <list>
#include <string>

#include "messages/server_server/append_entries/append_entries.h"
#include "messages/server_server/append_entries/append_entries_response.h"
#include "messages/server_server/request_vote/request_vote.h"
#include "messages/server_server/request_vote/request_vote_response.h"
#include "messages/statsCollector/leader_elected.h"
#include "messages/statsCollector/server_failure.h"
#include "messages/statsCollector/consensus_messages.h"
#include "messages/statsCollector/server_recovering.h"
#include "utils/printer.h"

void Server::refreshDisplay() const {
    if(test)return;
    ostringstream out;
    cDisplayString &dispStr = getParentModule()->getDisplayString();

    if (state == FOLLOWER)
        out << "i=block/circle;";
    if (state == LEADER)
        out << "i=block/triangle;";
    if (state == CANDIDATE)
        out << "i=block/square;";

    if (crashed)
        out << "i2=status/stop;";

    if (state == LEADER)
        out << "t=nextIndex: " << printElements(nextIndex, getParentModule()->getVectorSize())
                << endl << "matchIndex: "
                << printElements(matchIndex, getParentModule()->getVectorSize());

    dispStr.parse(out.str().c_str());
    if (getParentModule()->getParentModule()->par("displayLogs")){
        ostringstream labelText;
        labelText << padOut(to_string(getParentModule()->getIndex()), 3)
                << padOut(to_string(currentTerm), 6)
                << padOut(to_string(votedFor), 6)
                << padOut(to_string(commitIndex), 8)
                << padOut(to_string(lastApplied), 7) << "[" << log->toString()
                << "]";
        label->setText(labelText.str().c_str());
    }else{
        ostringstream labelText;
        labelText << padOut(to_string(getParentModule()->getIndex()), 3)
                << padOut(to_string(currentTerm), 6)
                << padOut(to_string(votedFor), 6)
                << padOut(to_string(commitIndex), 8);
       label->setText(labelText.str().c_str());
    }

    label->setPosition(cFigure::Point(20, 6 + getParentModule()->getIndex() * 6));
    label->setFont(cFigure::Font("Courier New"));
    label->setAnchor(cFigure::ANCHOR_NW);

    cCanvas *canvas = getParentModule()->getParentModule()->getCanvas();
    if (canvas->findFigure(label) != -1)
        canvas->removeFigure(label);
    canvas->addFigure(label);
}

void Server::initialize() {
    test= par("test");
    if(test){
        initializefortest();
        return;
    };
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    resendAppendEntryEvent = new cMessage("resendAppendEntryEvent");
    heartbeatEvent = new cMessage("heartbeatEvent");
    log = new LogEntryVector(getParentModule()->getIndex());
    nextIndex = new int[getServerNodeVectorSize()];
    matchIndex = new int[getServerNodeVectorSize()];
    nbOfServers = getParentModule()->getParentModule()->par("numServers");
    server_failure_probability = par("server_failure_probability");
    leader_failure_probability = par("leader_failure_Probability");
    channel_omission_probability = getParentModule()->getParentModule()->par("channel_omission_probability");

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

    cCanvas *canvas = getParentModule()->getParentModule()->getCanvas();
    canvas->addFigure(title);
}

void Server::finish() {
    delete[] nextIndex;
    delete[] matchIndex;
    if(!test){
    cancelAndDelete(electionTimeoutEvent);
    cancelAndDelete(heartbeatEvent);
    cancelAndDelete(recoverEvent);
    cancelAndDelete(resendAppendEntryEvent);
    cancelAndDelete(crashEvent);
    log->clear();
    }

}

void Server::handleMessage(cMessage *msg) {
    if (msg == crashEvent) {
        double theshold = state == LEADER ? 1-leader_failure_probability : 1-server_failure_probability;
        if (uniform(0, 1) > theshold){
            cancelEvent(crashEvent);
            scheduleCrash();
            return;
        }

            votedFor = -1;
            votesCount = 0;
            state = FOLLOWER;
            currentLeader= -1;

        getParentModule()->bubble("CRASHED");
        EV << "[Server" << getParentModule()->getIndex() << "] just crashed." << endl;
        crashed = true;
        cancelEvent(crashEvent);
        cancelEvent(heartbeatEvent);
        cancelEvent(electionTimeoutEvent);
        scheduleRecover();

        return;
    }

    if (msg == recoverEvent) {
        getParentModule()->bubble("RECOVERED");
        rescheduleElectionTimeout();
        crashed = false;
        scheduleCrash();
        ServerRecovering *server_recovered = new ServerRecovering(getParentModule()->getIndex(),getLastLogIndex());
        sendToStatsCollector(server_recovered);
        return;
    }

    if (crashed) {
        delete (msg);
        return;
    }

    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {
        if (msg == heartbeatEvent) {
            AppendEntries *heartbeat = new AppendEntries("Heartbeat",
                    currentTerm, getParentModule()->getIndex(), getLastLogIndex(),
                    getLastLogTerm(), { }, commitIndex);
            broadcast(heartbeat);
            scheduleHeartbeat();
            delete heartbeat;
        } else

        if (msg == electionTimeoutEvent) {
            ServerFailure *failed = new ServerFailure(true,currentTerm + 1);
            sendToStatsCollector(failed);
            startElection();
            ConsensusMessages *reqVotes = new ConsensusMessages(nbOfServers-1);
            sendToStatsCollector(reqVotes);
        }
        return;
    }

    // *** EXTERNAL MESSAGES ***
    // All messages from this point on are sent from other servers/clients

    // OMISSIONS OF THE CHANNEL
    // We simulate channel omissions, randomly deleting messages coming from the network
    double theshold =  1-channel_omission_probability;
    if (uniform(0, 1) > theshold){
        getParentModule()->bubble("Channel Omission");
        EV << "[Server " << getParentModule()->getIndex() << "] Message not received because of channel omission."<<endl;
        cancelAndDelete(msg);
        return;
    }

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
            AppendEntriesResponse *response = new AppendEntriesResponse(
                        currentTerm, false, true, getLastLogIndex());
                send(response, "out", msg->getArrivalGate()->getIndex());
            cancelAndDelete(msg);
            return;
        }
    }

    HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
    handableMsg->handleOnServer(this);

    cancelAndDelete(msg);
}

void Server::startElection() {
    electionTimeout = par("electionTimeout");
    scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
    currentTerm = currentTerm + 1;
    votesCount = 0; // Reset votes count from previous election

    EV << "[Server" << getParentModule()->getIndex() << "] Start election at " << simTime()
              << " , term = " << currentTerm << endl;

    state = CANDIDATE;
    votesCount++;
    votedFor = getParentModule()->getIndex();

    RequestVote *requestvote = new RequestVote("RequestVote", currentTerm,
            getParentModule()->getIndex(), getLastLogIndex(), getLastLogTerm());
    broadcast(requestvote);
    delete requestvote;
}

void Server::sendToStatsCollector(cMessage *msg){
    if (!(getParentModule()->getParentModule()->par("disableStatsCollector")))
        send(msg, "toStatsCollector");
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

void Server::scheduleCrash() {
    simtime_t crashTimeout = exponential(par("crashTimeoutAvg").doubleValue());
    scheduleAt(simTime() + crashTimeout, crashEvent);
}

void Server::scheduleRecover() {
    simtime_t recoverTimeout = exponential(par("recoverTimeoutAvg").doubleValue());
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

int Server::getServerNodeVectorSize(){
    return getParentModule()->getVectorSize();
}
void Server:: initializefortest(){
       log = new LogEntryVector(getParentModule()->getIndex());
       nextIndex = new int[1];
       matchIndex = new int[1];
       nextIndex[0] = 1;
       matchIndex[0] = 0;
       currentTerm=1;
       state= LEADER;
       nbOfServers = 2;
}
