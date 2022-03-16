package raft_project.simulations;

@license(LGPL);

simple Server
{
    parameters:
        double electionTimeout @unit(ms) = default(uniform(400ms, 500ms));
        double heartbeatPeriod @unit(ms) = electionTimeout/2;

        @display("i=block/process");
    gates:
        input in[];
        output out[];
}

network Raft
{
    parameters:
        int numServers = default(5);

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
    connections:
        for i=0..numServers-1, for j=i+1..numServers-1 {
            server[i].out++ --> Channel --> server[j].in++;
            server[i].in++ <-- Channel <-- server[j].out++;
        }
}
