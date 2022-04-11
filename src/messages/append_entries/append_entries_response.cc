#include "append_entries_response.h"
#include "append_entries.h"
#include "../ServerReplyToClient.h"
AppendEntriesResponse::AppendEntriesResponse(int term, bool success,int index) {
    cMessage::setName("AppendEntriesResponse");
    this->term = term;
    this->success = success;
    this->index=index;
}

void AppendEntriesResponse::handleOnServer(Server *server) const {

    if(term>server->currentTerm){
        server->currentTerm=term;
        server->currentState=FOLLOWER;
        server->votesCount=0;
        server->rescheduleElectionTimeout();
    }
    if(success) {
    	//appendentries succeded , updating matchindex
        if(index>server->matchIndex[getArrivalGate()->getIndex()])server->matchIndex[getArrivalGate()->getIndex()] =index;

        int  indexcount=0;
        for(int i=0;i<server->getVectorSize();i++){
        	if(server->matchIndex[i]>=index) indexcount++;
        }


        // update indexcommit
        if( index>server->commitIndex && indexcount>server->getVectorSize()/2 -1  )
                    {
                        server->commitIndex=index;
                        bool commitfound=false;
                        //sending to all clients response corresponding to new entry committed
                        for(int logindex=server->commitIndex - 1;logindex>=0 && !commitfound ;logindex--){
                            list<LogEntry> :: iterator it= server->log.begin();
                              advance(it,logindex);

                            if((*it).isCommitted()){
                              commitfound=true;
                            }
                            else{
                               (*it).setCommitted(true);
                                server->send( new ServerReplyToClient( true,server->getIndex(),(*it).getRequestId())
                                                        ,"toclients", (*it).getClientId());

                            }


                        }

                    }


        //appendentries succeded , updating nextindex
        if(server->nextIndex[getArrivalGate()->getIndex()]<=server->log.size()) server->nextIndex[getArrivalGate()->getIndex()]=server->nextIndex[getArrivalGate()->getIndex()]+1;







    }
    else{ // appendentries failed, adjust next index and retry appendentries
        server->nextIndex[getArrivalGate()->getIndex()]=server->nextIndex[getArrivalGate()->getIndex()]-1;
        list<LogEntry> :: iterator it= server->log.begin();
        advance(it,server->nextIndex[getArrivalGate()->getIndex()]-1);
        list<LogEntry> tosend = {*it};
        int lastlogterm;
        if(server->log.size()>1){
        list<LogEntry> :: iterator prevlogit= server->log.begin();
                advance(prevlogit,server->nextIndex[getArrivalGate()->getIndex()]-2);
                 lastlogterm= (*prevlogit).getLogterm();
        }
        else lastlogterm=0;
       server->send( new AppendEntries("AppendEntries", server->currentTerm,
                        server->getIndex(), server->nextIndex[getArrivalGate()->getIndex()]-1, lastlogterm,tosend,
                        server->commitIndex),"out", getArrivalGate()->getIndex());

    }

}

cMessage* AppendEntriesResponse::dup() const {
    return new AppendEntriesResponse(term, success,index);
}

int AppendEntriesResponse::getTerm() const {
    return term;
}

bool AppendEntriesResponse::getSuccess() const {
    return success;
}

