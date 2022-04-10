#include "append_entries.h"
#include "append_entries_response.h"
#include "../../utils/log_entry.h"
#include<iterator>
#include <list>


AppendEntries::AppendEntries(string name, int term, int leaderId,
        int prevLogIndex, int prevLogTerm, list<LogEntry> entries,
        int leaderCommit) {
    cMessage::setName(name.c_str());
    this->term = term;
    this->leaderId = leaderId;
    this->prevLogIndex = prevLogIndex;
    this->prevLogTerm=prevLogTerm;
    this->entries = entries;
    this->leaderCommit = leaderCommit;
}

void AppendEntries::handleOnServer(Server *server) const {
        if (!(server->currentState == LEADER))
            server->rescheduleElectionTimeout();

        //reject appendentries with stale term
               if(term<server->currentTerm)
                   {
                   server->send(new AppendEntriesResponse(server->currentTerm, false,server->log.size()) ,"out", getArrivalGate()->getIndex());
                   return;
                   }

        // Received an heartbeat while being Candidate: compare the terms
        // and accept heartbeat sender as Leader if its term > my currentTerm
        if (server->currentState == CANDIDATE && term >= server->currentTerm) {
            server->currentState = FOLLOWER;
            server->currentTerm = term;
            server->votesCount = 0;

        }
        //update follower term and convert a leader into a follower if receives a heartbeat from a leader with a higher term
        if(term>server->currentTerm ){
            server->currentState = FOLLOWER;
                        server->currentTerm = term;
                        server->votesCount = 0;
        }
        server->currentLeader=leaderId;

  //entries implemented as a list , in truth there will be maximum 1 entry at a time
        if(!entries.empty()){
            if(server->log.size()>=this->prevLogIndex){
                 list<LogEntry> :: iterator it =       server->log.begin();
                 advance(it,prevLogIndex-1);
                 LogEntry logtocheck = *it;     // finding the common entry with the leader
                 if(logtocheck.getLogterm()==term){ //if such entry exist...

                	 list<LogEntry> :: iterator tail = server->log.begin();
                	 advance(tail,server->log.size());
                	 list<LogEntry> entries_ = entries; //why doesn' t work if i put directly entries in splice below?
                	 server->log.erase(it,tail); //deleting all entries after the common entry with the leader
                     server->log.splice(server->log.end(),entries_); //adding new entries
                     server->send(new AppendEntriesResponse(server->currentTerm, true,server->log.size()) ,"out", getArrivalGate()->getIndex());  // no common entry found with leader
                                          return;
                 }
                 else { // common entry not found
                     server->send(new AppendEntriesResponse(server->currentTerm, false,server->log.size()) ,"out", getArrivalGate()->getIndex());  // no common entry found with leader
                     return;
                 }
            }
            else{ // no entry found at prevLogIndex
                server->send(new AppendEntriesResponse(server->currentTerm, false,server->log.size()) ,"out", getArrivalGate()->getIndex());
                                     return;
            }
        }
        //If leaderCommit > commitIndex, set commitIndex =   min(leaderCommit, index of last new entry)
            if(leaderCommit>server->commitIndex) {
                                                         int indexoflastentry = server->log.size();
                                                         if(indexoflastentry<leaderCommit) server->commitIndex=indexoflastentry;
                                                         else server->commitIndex=leaderCommit;

                                                     }




}

cMessage* AppendEntries::dup() const {
    return new AppendEntries(getName(), term, leaderId, prevLogIndex,
            prevLogTerm, entries, leaderCommit);
}

int AppendEntries::getTerm() const {
    return term;
}

int AppendEntries::getLeaderId() const {
    return leaderId;
}

int AppendEntries::getPrevLogIndex() const {
    return prevLogIndex;
}

int AppendEntries::getPrevLogTerm() const {
    return prevLogTerm;
}

list<LogEntry> AppendEntries::getEntries() const {
    return entries;
}

int AppendEntries::getLeaderCommit() const {
    return leaderCommit;
}
