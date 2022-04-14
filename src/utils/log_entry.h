#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <iostream>
#include <string>
using namespace std;
int square(int);

class LogEntry
{
  private:
   int logterm;
   string entry;
   bool committed;
   int requestId;
   int clientId;
   int index;

  public:
    LogEntry(int logterm,string entry,int requestId,int clientId,int index)
    {
        this->logterm=logterm;
        this->entry= entry;
        this->committed=false;
        this->requestId=requestId;
        this->clientId=clientId;
        this->index=index;

    }




    string getEntry() const {
        return entry;
    }

    void setEntry(string entry) {
        this->entry = entry;
    }

    int getLogTerm() const {
        return logterm;
    }

    void setLogterm(int logterm) {
        this->logterm = logterm;
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
