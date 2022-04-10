
// 

#ifndef CLIENT_H_
#define CLIENT_H_
#include <omnetpp.h>
#include <string>
using namespace omnetpp;
using namespace std;
class Client : public cSimpleModule {
public:
 int numberofservers;    //client that send a command , wait for the reply of it and then send another command and so on
 int lastrequestid;
 int numberofrequests;
 string lastcommand;
 cMessage *sendCommandEvent;
 double sendCommandPeriod;
 double resendCommandPeriod;
 cMessage *resendCommandEvent;
 void scheduleSendCommand();
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
private:
string random_string( int );

};
Define_Module(Client);

#endif /* CLIENT_H_ */
