
#include "Mock.h"
#include "messages/client_server/add_command.h"
#include "messages/server_server/append_entries/append_entries.h"
#include <assert.h>

void Mock::initialize() {

    send(new AddCommand(1, "test", 0), "out");
}
void Mock::handleMessage(cMessage *msg) {
    AppendEntries *handableMsg = check_and_cast<AppendEntries*>(msg);
    if(handableMsg->prevLogIndex!=0) getSimulation()->getActiveEnvir()->alert("FAIL, wrong prevLogIndex");
    if(handableMsg->prevLogTerm!=0)  getSimulation()->getActiveEnvir()->alert("FAIL, wrong prevLogIndex");
    if(handableMsg->entries.at(0).command!="test") getSimulation()->getActiveEnvir()->alert("FAIL, wrong command in the entry");
    delete msg;
}

void Mock::finish() {
std:: cout<<"PASSED"<<endl;
}




