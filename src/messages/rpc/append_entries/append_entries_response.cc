#include "../append_entries/append_entries.h"
#include "../append_entries/append_entries_response.h"
#include "../../add_command/add_command_response.h"

void AppendEntriesResponse::handleOnServer(Server *server) const {
    if (result) {
        // AppendEntries succeeded, update matchIndex
        if (index > server->matchIndex[getArrivalGate()->getIndex()])
            server->matchIndex[getArrivalGate()->getIndex()] = index;

        int indexcount = 0;
        for (int i = 0; i < server->getVectorSize(); i++) {
            if (server->matchIndex[i] >= index)
                indexcount++;
        }

        // Update indexCommit
        if (index > server->commitIndex
                && indexcount > server->getVectorSize() / 2 - 1) {
            server->commitIndex = index;
            bool commitfound = false;
            //sending to all clients response corresponding to new entry committed
            for (int logindex = server->commitIndex - 1;
                    logindex >= 0 && !commitfound; logindex--) {
                list<LogEntry>::iterator it = server->log.begin();
                advance(it, logindex);

                if ((*it).isCommitted()) {
                    commitfound = true;
                } else {
                    (*it).setCommitted(true);
                    server->send(
                            new AddCommandResponse(true, server->getIndex(),
                                    (*it).getRequestId()), "toclients",
                            (*it).getClientId());
                }
            }
        }

        // AppendEntries succeeded, update nextIndex
        if (server->nextIndex[getArrivalGate()->getIndex()]
                <= server->log.size())
            server->nextIndex[getArrivalGate()->getIndex()] =
                    server->nextIndex[getArrivalGate()->getIndex()] + 1;

    } else { // AppendEntries failed, adjust nextIndex and retry
        server->nextIndex[getArrivalGate()->getIndex()] =
                server->nextIndex[getArrivalGate()->getIndex()] - 1;
        list<LogEntry>::iterator it = server->log.begin();
        advance(it, server->nextIndex[getArrivalGate()->getIndex()] - 1);
        list<LogEntry> tosend = { *it };
        int lastlogterm;
        if (server->log.size() > 1) {
            list<LogEntry>::iterator prevlogit = server->log.begin();
            advance(prevlogit,
                    server->nextIndex[getArrivalGate()->getIndex()] - 2);
            lastlogterm = (*prevlogit).getLogTerm();
        } else
            lastlogterm = 0;
        server->send(
                new AppendEntries("AppendEntries", server->currentTerm,
                        server->getIndex(),
                        server->nextIndex[getArrivalGate()->getIndex()] - 1,
                        lastlogterm, tosend, server->commitIndex), "out",
                getArrivalGate()->getIndex());

    }

}
