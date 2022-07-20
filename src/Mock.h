

#ifndef MOCK_H_
#define MOCK_H_

#include <omnetpp.h>
#include <string>
#include <vector>
#include "messages/handable_message.h"
using namespace omnetpp;
using namespace std;


class Mock: public cSimpleModule {
public:
    string test_type;
    vector<string> tests  ={"add_command","append_entries_response_false"};
    vector<HandableMessage *> messages  ={};
    int testcount=0;
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;


};

Define_Module (Mock);


#endif /* MOCK_H_ */
