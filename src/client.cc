#include <ctime>
#include <random>

#include "client.h"
#include "messages/client_server/add_command.h"
#include "utils/unique_id.h"

void Client::initialize() {
    numberOfServers = getParentModule()->getParentModule()->par("numServers");
    channel_omission_probability = getParentModule()->getParentModule()->par("channel_omission_probability");
    resendCommandPeriod = par("resendCommandTimeout");
    sendCommandPeriod = par("sendCommandTimeout").doubleValue();

    sendCommandEvent = new cMessage("SendCommandEvent");
    resendCommandEvent = new cMessage("ResendCommandEvent");
    scheduleSendCommand();
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
            send(new AddCommand(lastCommandId, lastCommand, getParentModule()->getIndex()), "out",
                    serverindex);
            
            cancelResendCommandTimeout();
            simtime_t resendCommandTimeout = resendCommandPeriod;
            scheduleAt(simTime() + resendCommandTimeout, resendCommandEvent);
            if (!(getParentModule()->getParentModule()->par("disableStatsCollector"))){
                // Inform the StatsCollector of a new AddCommand request
                sendToStatsCollector(new AddCommand(lastCommandId, lastCommand, getParentModule()->getIndex()));
            }
        } else if (msg == resendCommandEvent) {
            int serverindex = uniform(0, numberOfServers - 1);
            send(new AddCommand(lastCommandId, lastCommand, getParentModule()->getIndex()), "out",
                    serverindex);

            scheduleResendCommand();           
        }
       
        return;
    }
    
    // OMISSIONS OF THE CHANNEL
    // We simulate channel omissions, randomly deleting messages coming from the network
    double theshold = 1 - channel_omission_probability;
    if (uniform(0, 1) > theshold){
        getParentModule()->bubble("Channel Omission");
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
    cModule *ref = getParentModule()->getParentModule()->getSubmodule("statsCollector");
    StatsCollector *statsCollector = check_and_cast<StatsCollector *>(ref);
    if (statsCollector == nullptr)
    {
        throw invalid_argument("Cannot retrieve toStatsCollector Module ");
    }
    if (!(getParentModule()->getParentModule()->par("disableStatsCollector")))
        statsCollector->emitCommandTimeResponse(simTime() - commandTimestamp, getParentModule()->getIndex());

}

void Client::sendToStatsCollector(cMessage *msg){
        send(msg, "toStatsCollector");
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

