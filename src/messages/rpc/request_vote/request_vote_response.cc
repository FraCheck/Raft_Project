#include "../../rpc/request_vote/request_vote_response.h"
#include "../../rpc/append_entries/append_entries.h"

void RequestVoteResponse::handleOnServer(Server *server) const {

    if (!result) {
        if (term > server->currentTerm) {
            server->currentTerm = term;
            server->currentState = FOLLOWER;
            server->votesCount = 0;
        }
        server->rescheduleElectionTimeout();
        return;
    }

    server->votesCount++;
    server->stopElectionTimeout();

    if (server->votesCount > server->getVectorSize() / 2) {
        server->currentState = LEADER;
        server->currentLeader = server->getIndex();
        for (int i = 0; i < server->getVectorSize(); i++) {
            server->nextIndex[i] = 1;
            server->matchIndex[i] = 0;
        }

        if (server->votesCount - 1 <= server->getVectorSize() / 2) {
            std::list<LogEntry> empty_log = { };
            AppendEntries *heartbeat = new AppendEntries("Heartbeat",
                    server->currentTerm, server->getIndex(),
                    server->getLastLogIndex(), server->getLastLogTerm(),
                    empty_log, server->commitIndex);
            server->broadcast(heartbeat);
            server->scheduleHeartbeat();
        }

    } else
        server->rescheduleElectionTimeout();

    return;
}
