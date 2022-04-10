
#include "ServerReplyToClient.h"
#include "ClientSendCommand.h"

ServerReplyToClient :: ServerReplyToClient(bool success, int leaderId,int requestId){
    cMessage::setName("ServerReplyToClient");
       this->success=success;
       this->leaderId=leaderId;
       this->requestId=requestId;
}
void ServerReplyToClient::handleOnClient(Client *client) const {
    client->cancelEvent(client->resendCommandEvent);
    if(!success){

        client->send(new ClientSendCommand(client->lastcommand,client->lastrequestid,client->getIndex()), "out", leaderId);
        simtime_t resendCommandTimeout = client->resendCommandPeriod;
                         client->scheduleAt(simTime() + resendCommandTimeout, client->resendCommandEvent );  //retry to contact the server after resendCommandperiod if it doesn't reply
                         return;
    }
    else{
        if(client->lastrequestid==requestId) client->scheduleSendCommand(); // schedule another sendcommandrequest with a new command
        else throw invalid_argument("received response for another request id") ; // this shouldn't happen
    }


}
cMessage* ServerReplyToClient::dup() const {
    return new ServerReplyToClient(success,leaderId, requestId);
}
