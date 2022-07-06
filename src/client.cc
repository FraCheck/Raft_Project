#include <ctime>
#include <random>

#include "client.h"
#include "messages/client_server/add_command.h"
#include "utils/unique_id.h"

void Client::initialize() {
    numberOfServers = getParentModule()->par("numServers");
    channel_omission_probability = getParentModule()->par("channel_omission_probability");
    resendCommandPeriod = par("resendCommandTimeout");
    sendCommandPeriod = par("sendCommandTimeout").doubleValue();

    sendCommandEvent = new cMessage("SendCommandEvent");
    resendCommandEvent = new cMessage("ResendCommandEvent");
    scheduleSendCommand();

    // Signals registering
    commandResponseTimeSignal = registerSignal("commandResponseTime");
}

void Client::finish() {
    cancelAndDelete(sendCommandEvent);
    cancelAndDelete(resendCommandEvent);
}

void Client::handleMessage(cMessage *msg) {
    // *** SELF-MESSAGES ***
    if (msg->isSelfMessage()) {
        if (msg == sendCommandEvent) {
            commandTimestamp = simTime();
            // Select randomly the recipient
            int serverindex = uniform(0, numberOfServers - 1);

            lastCommandId = UniqueID().id;
            lastCommand = buildRandomString(5);
            send(new AddCommand(lastCommandId, lastCommand, getIndex()), "out",
                    serverindex);
            
            cancelResendCommandTimeout();
            simtime_t resendCommandTimeout = resendCommandPeriod;
            scheduleAt(simTime() + resendCommandTimeout, resendCommandEvent);
        } else if (msg == resendCommandEvent) {
            int serverindex = uniform(0, numberOfServers - 1);
            send(new AddCommand(lastCommandId, lastCommand, getIndex()), "out",
                    serverindex);

            scheduleResendCommand();           
        }
       
        return;
    }
    
    // OMISSIONS OF THE CHANNEL
    // We simulate channel omissions, randomly deleting messages coming from the network
    double theshold =  1-channel_omission_probability;
    if (uniform(0, 1) > theshold){
        bubble("CHANNEL OMISSION");
        cancelAndDelete(msg);
        return;
    }

    HandableMessage *handableMsg = check_and_cast<HandableMessage*>(msg);
    handableMsg->handleOnClient(this);

    cancelAndDelete(msg);
}

void Client::scheduleSendCommand() {
    simtime_t sendCommandTimeout = exponential(sendCommandPeriod);
    scheduleAt(simTime() + sendCommandTimeout, sendCommandEvent);
}

void Client::cancelSendCommandTimeout() {
    cancelEvent(sendCommandEvent);
}

void Client::scheduleResendCommand() {
    simtime_t resendCommandTimeout = resendCommandPeriod;
    scheduleAt(simTime() + resendCommandTimeout, resendCommandEvent);
}

void Client::cancelResendCommandTimeout() {
    cancelEvent(resendCommandEvent);
}

void Client::emitCommandTimeResponseSignal() {
    emit(commandResponseTimeSignal, simTime() - commandTimestamp);
    EV << "[Client" << this->getIndex() << "] Emitted command execution response time: " << simTime() - commandTimestamp << endl;
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

