#include "ClientSendCommand.h"
#include "ServerReplyToClient.h"
#include "append_entries/append_entries.h"
ClientSendCommand::ClientSendCommand(string command, int idrequest,int clientId) {
    cMessage::setName("ClientSendCommand");
    this->command=command;
    this->idrequest = idrequest;
    this->clientId=clientId;
}
void ClientSendCommand::handleOnServer(Server *server) const {
 if ((!server->currentState)==LEADER)
 {
     server->send(new ServerReplyToClient(false, server->currentLeader,idrequest),"out", getArrivalGate()->getIndex());
     return;
 }
 bool logfound=false;
 for(int index=0; index<server->log.size() && !logfound;index++){
     list<LogEntry> :: iterator it= server->log.begin();
                                  advance(it,server->commitIndex-1);
                                LogEntry logtocheck= *it;
                                if(logtocheck.getRequestId()==idrequest){
                                    logfound=true;
                                    if(logtocheck.isCommitted())   server->send(new ServerReplyToClient(true, server->currentLeader,idrequest),"toclients", getArrivalGate()->getIndex());
                                }
 }
 if(!logfound){
     LogEntry *newentry = new LogEntry(server->currentTerm,command,idrequest,clientId,server->log.size()+1);
      server->log.push_back(*newentry);
      int lastlogterm;
       if(server->log.size()>1){
              list<LogEntry> :: iterator prevlogit= server->log.begin();
                      advance(prevlogit,server->log.size()-2);
                       lastlogterm= (*prevlogit).getLogterm();
       }
       else lastlogterm=0;
      server->broadcast(new AppendEntries("AppendEntries", server->currentTerm,
                          server->getIndex(), server->log.size()-1, lastlogterm, {server->log.back()},
                          server->commitIndex));
                  server->retryAppendEntryEvent = new cMessage("retryAppendEntryEvent");

                      simtime_t appendEntryPeriod = server->par("retryAppendEntriesPeriod");
                      server->scheduleAt(simTime() + appendEntryPeriod, server->retryAppendEntryEvent);
 }

}
cMessage* ClientSendCommand::dup() const {
    return new ClientSendCommand(command, idrequest,clientId);
}


