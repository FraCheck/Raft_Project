#include "delayQueue.h"
#include <string.h>

void DelayQueue::initialize() {
    numberOfServers = getParentModule()->getParentModule()->par("numServers");
    numberOfClients = getParentModule()->getParentModule()->par("numClients");
    avgServiceTime = par("avgServiceTime").doubleValue();
    isDelayQueueBusy = false;
    msgInService = endOfServiceMsg = nullptr;
    endOfServiceMsg = new cMessage("end-service");

}

void DelayQueue::finish() {
    delete msgInService;
    cancelAndDelete(endOfServiceMsg);
}

void DelayQueue::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) { // Message in service has been processed/delayed

        //log service completion


        send(msgInService, "out");
        //start next packet processing if queue not empty
        if (!queue.isEmpty()) {
            //Put the next message from the queue inside the server
            msgInService = (cMessage *)queue.pop();

            //start service
            startPacketService();
        } else {
            // DelayQueue is not busy anymore
            msgInService = nullptr;
            isDelayQueueBusy = false;

            //log idle server
            //EV << "Empty queue, DelayQueue goes IDLE" <<endl;
        }

    }
    else { // packet from other module has arrived

        // Setting arrival timestamp as msg field
        msg->setTimestamp();

        if (isDelayQueueBusy) {
            putPacketInQueue(msg);
        }
        else { // DelayQueue idle, start service right away
            // Put the message in service and start it
            msgInService = msg;
            startPacketService();

            //server is now busy
            isDelayQueueBusy=true;
        }
    }
}

void DelayQueue::startPacketService()
{

    //generate service time and schedule completion accordingly
    simtime_t serviceTime = exponential(avgServiceTime);
    scheduleAt(simTime()+serviceTime, endOfServiceMsg);

    //log service start
    //EV << "Starting service of " << msgInServer->getName() << endl;

}

void DelayQueue::putPacketInQueue(cMessage *msg)
{
    queue.insert(msg);

    //log new message in queue
    //EV << msg->getName() << " enters queue"<< endl;
}

