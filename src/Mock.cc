
#include "Mock.h"
#include "messages/client_server/add_command.h"
#include "messages/server_server/append_entries/append_entries.h"
#include "messages/server_server/append_entries/append_entries_response.h"
#include <assert.h>
#include "messages/test_messages/endtest.h"

void Mock::initialize() {
    test_type= par("test_type").stringValue();
    HandableMessage *msg = new AddCommand(1, "test", 0);
    messages.push_back(msg);
    if(test_type=="add_command")send(msg, "out");
    msg = new AppendEntriesResponse(2, true,false, 0);
    messages.push_back(msg);
    if(test_type=="append_entries_response_false")send(msg, "out");
    if(test_type=="all")send(messages.at(testcount),"out");
}
void Mock::handleMessage(cMessage *msg) {
    string actual_test_type;
    if(test_type=="all")actual_test_type=tests.at(testcount);
    else actual_test_type=test_type;
    if(actual_test_type=="add_command"){
    AppendEntries *handableMsg = check_and_cast<AppendEntries*>(msg);
    if(handableMsg->prevLogIndex!=0) getSimulation()->getActiveEnvir()->alert("FAIL add_command, wrong prevLogIndex");
    if(handableMsg->prevLogTerm!=0)  getSimulation()->getActiveEnvir()->alert("FAIL add_command, wrong prevLogIndex");
    if(handableMsg->entries.at(0).command!="test") getSimulation()->getActiveEnvir()->alert("FAIL add_command, wrong command in the entry");
    }
    else if(test_type=="append_entries_response_false"){
        AppendEntries *handableMsg = check_and_cast<AppendEntries*>(msg);
            if(handableMsg->prevLogIndex!=1) getSimulation()->getActiveEnvir()->alert("FAIL append_entries_response_false, wrong prevLogIndex");
            if(handableMsg->prevLogTerm!=1)  getSimulation()->getActiveEnvir()->alert("FAIL append_entries_response_false, wrong prevLogIndex");
            if(handableMsg->entries.at(0).command!="hello") getSimulation()->getActiveEnvir()->alert("FAIL, wrong command in the entry");
    }
    delete msg;
    if(test_type=="all"){
    if(testcount<tests.size()-1){
        testcount++;
        send(new EndTest(tests.at(testcount)),"out");
        send(messages.at(testcount),"out");
    }
    }
}

void Mock::finish() {
 if(test_type!="all") std:: cout<<"PASSED"<<endl;
 else std:: cout<<testcount + 1 << " tests PASSED"<<endl;
}




