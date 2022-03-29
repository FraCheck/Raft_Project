#ifndef SERVER_H_
#define SERVER_H_

#include <typeinfo>
#include <list>
#include <string>

#include "messages/append_entries/append_entries.h"
#include "messages/append_entries/append_entries_response.h"
#include "messages/request_vote/request_vote.h"
#include "messages/request_vote/request_vote_response.h"

#include "utils/log_entry.h"

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
