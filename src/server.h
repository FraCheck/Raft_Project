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

    cMessage *electionTimeoutEvent; // message for election timeout event
    cMessage *heartbeatEvent;       // message for heartbeat event
    cMessage *retryAppendEntryEvent;  //message for retry appendentries to all the server not consistent with the leader log yet

    int votesCount = 0;
    bool faultywhenleader;
    bool crashed = false;
    simtime_t electionTimeout;

    // Persistent state on all servers
    int currentTerm = 0; // Latest term server has seen (0 on first boot, increases monotonically)
    int votedFor = -1; // CandidateId that received vote in current term (or -1 if none)
    list<LogEntry> log = { }; // Log entries

    // Volatile state on all servers
    int commitIndex = 0; // Index of highest log entry known to be committed (initialized to 0, increases monotonically)
    int lastApplied = 0; // Index of highest log entry applied to state machine (initialized to 0, increases monotonically)

    int *nextIndex;     // for each server, index of the next log entry  to send to that server (initialized to leader last log index + 1)
    int *matchIndex;   //for each server, index of highest log entry known to be replicated on server(even if not committed ) (initialized to 0, increases monotonically)
    int currentLeader; // current leader index
    void startElection();
    void scheduleHeartbeat();
    void rescheduleElectionTimeout();

    void broadcast(cMessage *msg);
    int getLastLogTerm();
    int getLastLogIndex();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(Server);

#endif
