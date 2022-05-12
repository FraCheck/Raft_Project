#ifndef SERVER_H_
#define SERVER_H_

#include <omnetpp.h>
#include <map>

#include "utils/log_entry.h"

using namespace omnetpp;
using namespace std;

enum ServerState {
    LEADER, FOLLOWER, CANDIDATE
};

class Server: public cSimpleModule {
public:
    ServerState state = FOLLOWER;

    // *** PERSISTENT STATE ON ALL SERVERS ***
    // (Updated on stable storage before responding to RPCs)

    // Latest term "seen" by the server (increases monotonically)
    int currentTerm = 0;

    // CandidateId that received vote in current term (or -1 if none)
    int votedFor = -1;

    // Log entries
    LogEntryVector *log = new LogEntryVector(getIndex());

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
    int *nextIndex = new int(getVectorSize());

    // For each server, index of the last log entry known to be replicated
    // (initialized to 0, increases monotonically)
    int *matchIndex = new int(getVectorSize());

    int currentLeader;
    int votesCount = 0;
    bool canFail;
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

    // Messages to trigger the server crash and recovery
    // (useless if canCrash = false)
    cMessage *crashEvent;
    cMessage *recoverEvent;

    void scheduleCrash();
    void scheduleRecover();

    cLabelFigure *label = new cLabelFigure("label");
    void refreshDisplay() const override;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(Server);

#endif
