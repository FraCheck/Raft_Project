
// 

#include "Client.h"
#include <ctime>
#include <random>
#include "messages/ClientSendCommand.h"
void Client::initialize() {
   numberofservers=  par("numServers");
   numberofrequests = 0;
   resendCommandPeriod=par("resendCommandTimeout");
   sendCommandPeriod=par("sendCommandTimeout");
   scheduleSendCommand();
}
void Client::scheduleSendCommand() {
    sendCommandEvent = new cMessage("sendCommandEvent");

    simtime_t sendCommandTimeout =sendCommandPeriod ;
    scheduleAt(simTime() + sendCommandTimeout, sendCommandEvent );
}
void Client::finish() {

}

void Client::handleMessage(cMessage *msg) {
  if(msg->isSelfMessage()){
    if (msg ==sendCommandEvent) {
        int serverindex = uniform(0,numberofservers);
        // unique id from 2 numbers x,y -> z z -> x,y  z = (x+y)(x+y+1)/2 + y
        //good if client doesn't crash , if it crashes it is necessary to give him a new index, or  another unique serial number algorithm that supports failures has to be implemented
              lastrequestid= (getIndex()+ numberofrequests) * (getIndex()+ numberofrequests + 1 )/2;
              lastcommand=random_string(5);
             send(new ClientSendCommand(lastcommand,lastrequestid,getIndex()), "out", serverindex);
              resendCommandEvent = new cMessage("ResendCommandEvent");
              numberofrequests++;
                  simtime_t resendCommandTimeout = resendCommandPeriod;
                  scheduleAt(simTime() + resendCommandTimeout, resendCommandEvent );

    }
    if(msg==resendCommandEvent){
        int serverindex = uniform(0,numberofservers);
        send(new ClientSendCommand(lastcommand,lastrequestid,getIndex()), "out", serverindex);
                    resendCommandEvent = new cMessage("ResendCommandEvent");
                        simtime_t resendCommandTimeout = resendCommandPeriod;
                        scheduleAt(simTime() + resendCommandTimeout, resendCommandEvent );
    }
  }
  else{
      HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
      handableMsg->handleOnClient(this);
      }
}
string Client::random_string( int length )
{
    static const string alphabet = "abcdefghijklmnopqrstuvwxyz" ;
    static default_random_engine rng( time(nullptr) ) ;
    static uniform_int_distribution<size_t> distribution( 0, alphabet.size() - 1 ) ;

    string str ;
    while( str.size() < length ) str += alphabet[ distribution(rng) ] ;
    return str ;
}

