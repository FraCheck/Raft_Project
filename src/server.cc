#include "server.h"
#include "messages/append_entries/append_entries.h"
#include "messages/append_entries/append_entries_response.h"
#include "messages/request_vote/request_vote.h"
#include "messages/request_vote/request_vote_response.h"
#include <typeinfo>
#include <list>
#include <string>

void Server::initialize() {
    nextIndex=  new int(getVectorSize());
    matchIndex= new int(getVectorSize());
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    retryAppendEntryEvent = new cMessage("retryAppendEntryEvent");
    heartbeatEvent = new cMessage("heartbeatEvent");
    faultywhenleader = par("faultyWhenLeader");
    electionTimeout = par("electionTimeout");
    simtime_t electionTimeout = par("electionTimeout");
    scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);


    WATCH(currentTerm);
    WATCH(currentState);
    WATCH(votedFor);
    WATCH(electionTimeout);
}

void Server::finish() {

    cancelEvent(electionTimeoutEvent);
    cancelEvent(retryAppendEntryEvent);
    cancelEvent(heartbeatEvent);
    delete electionTimeoutEvent;
    delete retryAppendEntryEvent;
    delete heartbeatEvent;



    EV << "[Server" << getIndex() << "] Votes count is " << votesCount << endl;


}

void Server::handleMessage(cMessage *msg) {

    if (crashed) {
        delete msg;
        return;
    }
    if(msg->isSelfMessage()){

    if (msg == heartbeatEvent) {
        std::list<LogEntry> empty_log = { };
        AppendEntries *heartbeat = new AppendEntries("Heartbeat", currentTerm,
                getIndex(), getLastLogIndex(), getLastLogTerm(), empty_log,
                commitIndex);
        broadcast(heartbeat);

        // Test what happens if a leader do not send HeartBeats to other servers anymore
        if (faultywhenleader && uniform(0, 1) > 0.7) {
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
    if(msg==retryAppendEntryEvent){  //resend to all servers the log they need to be consitent with the leader
        cancelEvent(retryAppendEntryEvent);
        bool allserversconsistent=true;
        for(int serverindex=0;serverindex<getVectorSize();serverindex++){
            if(log.size()>=nextIndex[serverindex]){
                allserversconsistent=false;
                list<LogEntry> :: iterator it= log.begin();
                        advance(it,nextIndex[serverindex]-1);
                       list<LogEntry> tosend = {*it};
                      send( new AppendEntries("AppendEntries", currentTerm,
                                       getIndex(), getLastLogIndex(), getLastLogTerm(),tosend,
                                       commitIndex),"out", serverindex);
            }
        }
        if(!allserversconsistent) { //retry appendentries until all servers are consistent with the log of the leader


                             simtime_t appendEntryPeriod = par("retryAppendEntriesPeriod");
                             scheduleAt(simTime() + appendEntryPeriod, retryAppendEntryEvent);
        }
    return;
    }
    }

    else{
        EV << "[Server" << getIndex() << "] Message received from Server"
                            << msg->getSenderModule()->getIndex() << " ~ " << msg->getName()
                            << endl;
    HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
    handableMsg->handleOnServer(this);
    }



}

void Server::startElection() {
    electionTimeout = uniform(400, 500);
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

void Server::rescheduleElectionTimeout() {
    cancelEvent(electionTimeoutEvent);
    simtime_t electionTimeout = par("electionTimeout");
    scheduleAt(simTime() + electionTimeout, electionTimeoutEvent);
}

void Server::broadcast(cMessage *msg) {

    for (int i = 0;  i < gateSize("out"); i++) {
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

