#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <iostream>
#include <string>

using namespace std;

int square(int);

class LogEntry {
private:
    // A state machine command
    string command;

    // Term during which the command was received by the leader
    int term;

    // Position of the LogEntry in the log
    int index;

    // True if the LogEntry is replicated on the majority of the servers
    bool committed;

    //
    int requestId;

    //
    int clientId;

public:
    LogEntry(int logterm, string command, int requestId, int clientId,
            int index) {
        this->term = logterm;
        this->command = command;
        this->committed = false;
        this->requestId = requestId;
        this->clientId = clientId;
        this->index = index;

    }

    string getEntry() const {
        return command;
    }

    void setEntry(string entry) {
        this->command = entry;
    }

    int getLogTerm() const {
        return term;
    }

    void setLogterm(int logterm) {
        this->term = logterm;
    }

    int getClientId() const {
        return clientId;
    }

    bool isCommitted() const {
        return committed;
    }

    void setCommitted(bool committed) {
        this->committed = committed;
    }

    int getRequestId() const {
        return requestId;
    }

    int getIndex() const {
        return index;
    }
};

#endif
