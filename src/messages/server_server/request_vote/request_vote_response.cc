#include "../../server_server/request_vote/request_vote_response.h"

#include "../../server_server/append_entries/append_entries.h"

void RequestVoteResponse::handleOnServer(Server *server) const {
    if (!result) {
        server->rescheduleElectionTimeout();
        return;
    }

    // Vote granted

    server->votesCount++;
    server->stopElectionTimeout();

    // "A candidate wins an election if it receives votes from
    // a majority of the servers in the full cluster for the same term."

    // Check if votes are enough
    if (server->votesCount <= server->getVectorSize() / 2) {
        server->rescheduleElectionTimeout();
        return;
    }

    server->currentState = LEADER;
    server->currentLeader = server->getIndex();

    // "When a leader first comes to power, it initializes all nextIndex values
    // to the index just after the last one in its log."

    for (int i = 0; i < server->getVectorSize(); i++) {
        server->nextIndex[i] = 1;
        server->matchIndex[i] = 0;
    }

    if (server->votesCount - 1 <= server->getVectorSize() / 2) {
        AppendEntries *heartbeat = new AppendEntries("Heartbeat",
                server->currentTerm, server->getIndex(),
                server->getLastLogIndex(), server->getLastLogTerm(), { },
                server->commitIndex);
        server->broadcast(heartbeat);
        server->scheduleHeartbeat();
    }
}
