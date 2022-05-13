#include <ctime>
#include <random>

#include "client.h"
#include "messages/client_server/add_command.h"
#include "utils/unique_id.h"

void Client::initialize() {
    numberOfServers = par("numServers");
    resendCommandPeriod = par("resendCommandTimeout");
    sendCommandPeriod = par("sendCommandTimeout");
    scheduleSendCommand();

    // Signals registering
    commandResponseTimeSignal = registerSignal("commandResponseTime");
}

void Client::finish() {
}

void Client::handleMessage(cMessage *msg) {
    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {
        if (msg == sendCommandEvent) {
            command_timestamp = simTime();
            // Select randomly the recipient
            int serverindex = uniform(0, numberOfServers - 1);

            lastCommandId = UniqueID().id;
            lastCommand = buildRandomString(5);
            send(new AddCommand(lastCommandId, lastCommand, getIndex()), "out",
                    serverindex);

            resendCommandEvent = new cMessage("ResendCommandEvent");
            simtime_t resendCommandTimeout = resendCommandPeriod;
            scheduleAt(simTime() + resendCommandTimeout, resendCommandEvent);
        }

        if (msg == resendCommandEvent) {
            int serverindex = uniform(0, numberOfServers - 1);
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

void Client::emitCommandTimeResponseSignal() {
    emit(commandResponseTimeSignal, simTime() - command_timestamp);
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

