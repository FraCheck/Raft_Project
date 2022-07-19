
#include "Mock.h"
#include "messages/client_server/add_command.h"
#include "messages/server_server/append_entries/append_entries.h"
#include "messages/server_server/append_entries/append_entries_response.h"
#include <assert.h>

void Mock::initialize() {
    test_type= par("test_type").stringValue();
    if(test_type=="add_command")send(new AddCommand(1, "test", 0), "out");
    else if(test_type=="append_entries_response_false")send(new AppendEntriesResponse(2, true,false, 0), "out");
}
void Mock::handleMessage(cMessage *msg) {
    if(test_type=="add_command"){
    AppendEntries *handableMsg = check_and_cast<AppendEntries*>(msg);
    if(handableMsg->prevLogIndex!=0) getSimulation()->getActiveEnvir()->alert("FAIL, wrong prevLogIndex");
    if(handableMsg->prevLogTerm!=0)  getSimulation()->getActiveEnvir()->alert("FAIL, wrong prevLogIndex");
    if(handableMsg->entries.at(0).command!="test") getSimulation()->getActiveEnvir()->alert("FAIL, wrong command in the entry");
    }
    else if(test_type=="append_entries_response_false"){
        AppendEntries *handableMsg = check_and_cast<AppendEntries*>(msg);
            if(handableMsg->prevLogIndex!=1) getSimulation()->getActiveEnvir()->alert("FAIL, wrong prevLogIndex");
            if(handableMsg->prevLogTerm!=1)  getSimulation()->getActiveEnvir()->alert("FAIL, wrong prevLogIndex");
            if(handableMsg->entries.at(0).command!="hello") getSimulation()->getActiveEnvir()->alert("FAIL, wrong command in the entry");
    }
    delete msg;
}

void Mock::finish() {
std:: cout<<"PASSED"<<endl;
}




