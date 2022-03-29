#ifndef MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_RESPONSE_H_
#define MESSAGES_APPEND_ENTRIES_APPEND_ENTRIES_RESPONSE_H_

#include "../handable_message.h"

class AppendEntriesResponse: public HandableMessage {
private:
    int term;
    bool success;

public:
    AppendEntriesResponse(int term, bool success);

    void handleOnServer(Server* server) const override;
    cMessage* dup() const override;

    // Getter methods
    int getTerm() const;
    bool getSuccess() const;
};

#endif
