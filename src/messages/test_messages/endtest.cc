

#include "endtest.h"

EndTest::EndTest(string test_type)
         {
    this->test_type = test_type;
}
void EndTest::handleOnServer(Server *server) const {
 server->test_type=test_type;
 server->initializefortest();
}
