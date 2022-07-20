

#ifndef MESSAGES_TEST_MESSAGES_ENDTEST_H_
#define MESSAGES_TEST_MESSAGES_ENDTEST_H_
#include "../handable_message.h"
#include "../../server.h"
#include <string>

class EndTest: public HandableMessage {
public:
    string test_type;

    EndTest(string test_type);
    void handleOnServer(Server *server) const override;
   // cMessage* dup() const override;
};

#endif /* MESSAGES_TEST_MESSAGES_ENDTEST_H_ */
