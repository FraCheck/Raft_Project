#include "append_entries_response.h"

AppendEntriesResponse::AppendEntriesResponse(int term, bool success) {
    cMessage::setName("AppendEntriesResponse");
    this->term = term;
    this->success = success;
}

void AppendEntriesResponse::handleOnServer(Server *server) const {
    // TODO: implement behavior
}

cMessage* AppendEntriesResponse::dup() const {
    return new AppendEntriesResponse(term, success);
}

int AppendEntriesResponse::getTerm() const {
    return term;
}

bool AppendEntriesResponse::getSuccess() const {
    return success;
}

