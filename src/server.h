#ifndef SERVER_H_
#define SERVER_H_

#include <omnetpp.h>
#include "utils/log_entry.h"
#include <map>
using namespace omnetpp;
using namespace std;

enum ServerState {
    LEADER, FOLLOWER, CANDIDATE
};

class Server: public cSimpleModule {
public:
    ServerState currentState = FOLLOWER;

    // *** PERSISTENT STATE ON ALL SERVERS ***
    // (Updated on stable storage before responding to RPCs)

    // Latest term "seen" by the server (increases monotonically)
    int currentTerm = 0;

    // CandidateId that received vote in current term (or -1 if none)
    int votedFor = -1;

    // Log entries
    list<LogEntry> log = { };

    // *** VOLATILE STATE ON ALL SERVERS ***

    // Index of the last log entry known to be committed
    // (increases monotonically)
    int commitIndex = 0;

    // Index of the last log entry applied to state machine
    // (increases monotonically)
    int lastApplied = 0;

    // *** VOLATILE STATE ON LEADERS ***
    // (Reinitialized after each election)

    // For each server, index of the next LogEntry to send
    // (initialized to leader last log index + 1)
    int *nextIndex;

    // For each server, index of the last log entry known to be replicated
    // (initialized to 0, increases monotonically)
    int *matchIndex;

    int currentLeader;
    int votesCount = 0;
    bool faultywhenleader;
    bool crashed = false;
    simtime_t electionTimeout;

    void startElection();
    void scheduleHeartbeat();
    void cancelHeartbeat();
    void rescheduleElectionTimeout();
    void stopElectionTimeout();
    void scheduleResendAppendEntries();
    void cancelResendAppendEntries();
    void broadcast(cMessage *msg);
    int getLastLogTerm();
    int getLastLogIndex();

private:
    // Message to trigger the election timeout
    cMessage *electionTimeoutEvent;

    // Message to trigger the sending of an heartbeat
    cMessage *heartbeatEvent;

    // Message to trigger the re-sending of appendentries
    // to all the server not yet consistent with the leader log
    cMessage *resendAppendEntryEvent;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(Server);

#endif
