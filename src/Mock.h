

#ifndef MOCK_H_
#define MOCK_H_

#include <omnetpp.h>
#include <string>
using namespace omnetpp;
using namespace std;


class Mock: public cSimpleModule {
public:
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;


};

Define_Module (Mock);


#endif /* MOCK_H_ */
