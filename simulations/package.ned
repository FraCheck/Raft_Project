package raft_project.simulations;

@license(LGPL);

simple Server
{
    parameters:
        volatile double electionTimeout @unit(ms) = default(uniform(400ms,500ms));
        volatile double heartbeatPeriod @unit(ms) = electionTimeout/2;
        double retryAppendEntriesPeriod @unit(ms) = default(150ms);
        bool faultyWhenLeader = default(false);
    gates:
        input in[];
        output out[];
        input fromclients[];
        output toclients[];
}
simple Client
{
    parameters:
        double sendCommandTimeout @unit(ms) = default(uniform(800ms, 1000ms));
        double resendCommandTimeout @unit(ms) = default(500ms);
        int numServers = default(5);
        @display("i=block/source");
    gates:
        input in[];
        output out[];
}

network Raft
{
    parameters:
        int numServers = default(5);
        int numClients = default(1);

        // The system should satisfies the following timing requirement:
		// broadcastTime << electionTimeout << MTBF
        double broadcastTime @unit(ms) = default(uniform(10ms, 20ms));
        double MTBF @unit(d) = default(uniform(100d, 200d));

    types:
        channel Channel extends ned.DelayChannel
        {
            delay = broadcastTime/2;
        }
    submodules:
        server[numServers]: Server;
        client[numClients]: Client;
    connections:
        for i=0..numServers-1, for j=i+1..numServers-1 {
            server[i].out++ --> Channel --> server[j].in++;
            server[i].in++ <-- Channel <-- server[j].out++;
        }
        for i=0..numClients-1, for j=0..numServers-1 {
            client[i].out++ --> Channel --> server[j].fromclients++;
            client[i].in++ <-- Channel <-- server[j].toclients++;
        }
}
