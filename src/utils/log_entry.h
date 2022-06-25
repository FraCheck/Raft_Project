#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <iostream>
#include <string>

using namespace std;
using namespace omnetpp;

int square(int);

class LogEntry {
public:
    // Term during which the command was received by the leader
    int term;

    // Unique identifier of the command
    int commandId;

    // A state machine command
    string command;

    // Position of the LogEntry in the log
    int index;

    // True if the LogEntry is replicated on the majority of the servers
    bool isCommitted;

    //
    int clientId;

    LogEntry(int term, string command, int commandId, int clientId, int index) {
        this->term = term;
        this->command = command;
        this->isCommitted = false;
        this->commandId = commandId;
        this->clientId = clientId;
        this->index = index;
    }

    LogEntry(LogEntry *logEntry) {
        this->term = logEntry->term;
        this->command = logEntry->command;
        this->isCommitted = logEntry->isCommitted;
        this->commandId = logEntry->commandId;
        this->clientId = logEntry->clientId;
        this->index = logEntry->index;
    }

};

// Class to implement the expected behavior of the log.
// In particular, its indexes starts from 1 and not from 0.
class LogEntryVector {
private:
    vector<LogEntry> log;
    int serverId;
public:
    LogEntryVector(int serverId) {
        this->serverId = serverId;
    }

    LogEntry getFromIndex(int index) const {
        if (index < 1 || index > log.size())
            throw invalid_argument(
                    "No log entries with index " + to_string(index));

        return log[index - 1];
    }

    LogEntry getLast() const {
        if (log.size() < 1)
            throw out_of_range("Not enough entries in the log");

        return log.end()[-1];
    }

    LogEntry getSecondToLast() const {
        if (log.size() < 2)
            throw out_of_range("Not enough entries in the log");

        return log.end()[-2];
    }

    int size() const {
        return log.size();
    }

    void append(LogEntry logEntry) {
        string::size_type index = logEntry.index;
        if (logEntry.index != log.size() + 1)
            throw invalid_argument(
                    "Cannot append a log entry with index "
                            + to_string(logEntry.index) + " (should be "
                            + to_string(log.size() + 1) + ")");

        log.push_back(logEntry);
    }

    void eraseStartingFromIndex(int index) {
        auto first = log.cbegin() + index - 1;
        auto last = log.cend();

        log.erase(first, last);
    }

    string toString() const {
        ostringstream out;

        for (LogEntry logEntry : log)
            out << " " << logEntry.term << "-" << logEntry.index << "-" << logEntry.command;

        return out.str();
    }
    void commit(int index) {
        log[index - 1].isCommitted = true;
    }
};

#endif
