
#include "Mock.h"
#include "messages/client_server/add_command.h"
#include "messages/server_server/append_entries/append_entries.h"
#include "messages/server_server/append_entries/append_entries_response.h"
#include "messages/server_server/request_vote/request_vote_response.h"
#include "messages/server_server/request_vote/request_vote.h"
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
    msg = new RequestVote("RequestVote",1,2,0,0);
    messages.push_back(msg);
    if(test_type=="request_vote_to_candidate")send(msg, "out");
    msg = new RequestVote("RequestVote",2,2,0,0);
    messages.push_back(msg);
    if(test_type=="request_vote_to_candidate_stale_term")send(msg, "out");
    vector<LogEntry> log  ={};
    LogEntry entry1 = new LogEntry(1,"hi",1,1,1);
    log.push_back(entry1);
    msg = new AppendEntries("AppendEntries",2,1,2,2,log,2);
    messages.push_back(msg);
    if(test_type=="append_entries_with_more_entry_in_log")send(msg,"out");
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
    else if(actual_test_type=="append_entries_response_false"){
        AppendEntries *handableMsg = check_and_cast<AppendEntries*>(msg);
            if(handableMsg->prevLogIndex!=1) getSimulation()->getActiveEnvir()->alert("FAIL append_entries_response_false, wrong prevLogIndex");
            if(handableMsg->prevLogTerm!=1)  getSimulation()->getActiveEnvir()->alert("FAIL append_entries_response_false, wrong prevLogIndex");
            if(handableMsg->entries.at(0).command!="hello") getSimulation()->getActiveEnvir()->alert("FAIL, wrong command in the entry");
    }
    else if(actual_test_type=="request_vote_to_candidate"){
        RequestVoteResponse *handableMsg = check_and_cast<RequestVoteResponse*>(msg);
        if(handableMsg->term!=1)getSimulation()->getActiveEnvir()->alert("FAIL request_vote_to_candidate, wrong term");
        if(handableMsg->result)getSimulation()->getActiveEnvir()->alert("FAIL request_vote_to_candidate, vote granted when it shouldn't");
    }
    else if(actual_test_type=="request_vote_to_candidate_stale_term"){
        RequestVoteResponse *handableMsg = check_and_cast<RequestVoteResponse*>(msg);
               if(handableMsg->term!=2)getSimulation()->getActiveEnvir()->alert("FAIL request_vote_to_candidate_stale_term, wrong term");
               if(!handableMsg->result)getSimulation()->getActiveEnvir()->alert("FAIL request_vote_to_candidate_stale_term, vote  not granted when it should be");
    }
    else if(actual_test_type=="append_entries_with_more_entry_in_log"){
        AppendEntriesResponse *handableMsg = check_and_cast<AppendEntriesResponse*>(msg);
                      if(handableMsg->term!=2)getSimulation()->getActiveEnvir()->alert("FAIL append_entries_with_more_entry_in_log, wrong term");
                      if(handableMsg->result)getSimulation()->getActiveEnvir()->alert("FAIL append_entries_with_more_entry_in_log, response should be false");
                      if(handableMsg->lastLogIndex!=0)getSimulation()->getActiveEnvir()->alert("FAIL append_entries_with_more_entry_in_log, wrong lastlogindex of follower");
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




