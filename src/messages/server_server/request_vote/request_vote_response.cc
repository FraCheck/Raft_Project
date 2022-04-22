#include "../request_vote/request_vote_response.h"
#include "../append_entries/append_entries.h"

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

    if (server->state == LEADER)
        return;

    server->state = LEADER;
    server->currentLeader = server->getIndex();

    EV << endl << "NEW LEADER: Server #" << server->getIndex() << endl;

    // "When a leader first comes to power, it initializes all nextIndex values
    // to the index just after the last one in its log."

    for (int i = 0; i < server->getVectorSize(); i++) {
        server->nextIndex[i] = server->getLastLogIndex() + 1;
        server->matchIndex[i] = 0;
    }

    server->logNextAndMatchIndexes();

    AppendEntries *heartbeat = new AppendEntries("Heartbeat",
            server->currentTerm, server->getIndex(), server->getLastLogIndex(),
            server->getLastLogTerm(), { }, server->commitIndex);

    server->broadcast(heartbeat);
    server->scheduleHeartbeat();
}
