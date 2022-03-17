



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

  public:
    LogEntry(int logterm,string entry)
    {
        this->logterm=logterm;
        this->entry= entry;
        this->committed=false;
    }

    bool isCommitted() const {
        return committed;
    }

    void setCommitted(bool committed) {
        this->committed = committed;
    }

    string getEntry() const {
        return entry;
    }

    void setEntry(string entry) {
        this->entry = entry;
    }

    int getLogterm() const {
        return logterm;
    }

    void setLogterm(int logterm) {
        this->logterm = logterm;
    }
};




