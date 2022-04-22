#include <ctime>
#include <random>
#include "client.h"

#include "messages/client_server/add_command.h"

void Client::initialize() {
    numberOfServers = par("numServers");
    numberOfRequests = 0;
    resendCommandPeriod = par("resendCommandTimeout");
    sendCommandPeriod = par("sendCommandTimeout");
    scheduleSendCommand();
}

void Client::finish() {
}

void Client::handleMessage(cMessage *msg) {
    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {
        if (msg == sendCommandEvent) {
            // Select randomly the recipient
            int serverindex = uniform(0, numberOfServers);

            // Generate the requestId
            // unique id from 2 numbers x,y -> z z -> x,y  z = (x+y)(x+y+1)/2 + y
            // good if client doesn't crash , if it crashes it is necessary to give him a new index, or  another unique serial number algorithm that supports failures has to be implemented
            lastCommandId = (getIndex() + numberOfRequests)
                    * (getIndex() + numberOfRequests + 1) / 2;

            lastCommand = buildRandomString(5);
            send(new AddCommand(lastCommandId, lastCommand, getIndex()), "out",
                    serverindex);
            resendCommandEvent = new cMessage("ResendCommandEvent");
            numberOfRequests++;
            simtime_t resendCommandTimeout = resendCommandPeriod;
            scheduleAt(simTime() + resendCommandTimeout, resendCommandEvent);
        }

        if (msg == resendCommandEvent) {
            int serverindex = uniform(0, numberOfServers);
            send(new AddCommand(lastCommandId, lastCommand, getIndex()), "out",
                    serverindex);

            scheduleResendCommand();
        }

        return;
    }

    HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
    handableMsg->handleOnClient(this);
}

void Client::scheduleSendCommand() {
    sendCommandEvent = new cMessage("SendCommandEvent");

    simtime_t sendCommandTimeout = sendCommandPeriod;
    scheduleAt(simTime() + sendCommandTimeout, sendCommandEvent);
}

void Client::scheduleResendCommand() {
    resendCommandEvent = new cMessage("ResendCommandEvent");

    simtime_t resendCommandTimeout = resendCommandPeriod;
    scheduleAt(simTime() + resendCommandTimeout, resendCommandEvent);
}

void Client::cancelResendCommandTimeout() {
    cancelEvent(resendCommandEvent);
}

string Client::buildRandomString(int length) {
    static const string alphabet = "abcdefghijklmnopqrstuvwxyz";
    static default_random_engine rng(time(nullptr));
    static uniform_int_distribution<size_t> distribution(0,
            alphabet.size() - 1);

    string str;
    while (str.size() < length)
        str += alphabet[distribution(rng)];
    return str;
}

