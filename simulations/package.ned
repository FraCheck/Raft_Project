package raft_project.simulations;

@license(LGPL);




simple Server
{
    parameters:
        volatile double electionTimeout @unit(s) = default(uniform(0.4s,0.5s));
        volatile double heartbeatPeriod @unit(s) = default(uniform(0.2s,0.25s));
        double server_failure_probability = default(0.2);
        double leader_failure_Probability = default(0.2);
        int numServers = default(5);
        int numClients = default(1);
        bool canFail = default(false);
        double crashTimeout @unit(s) = default(uniform(1s, 3s));
        double recoverTimeout @unit(s) = default(2s);
		
    gates:
        input in[];
        output out[];
        output toclients[];
        input fromclients[];
        output toStatsCollector;
}

simple Client
{
    parameters:
        volatile double sendCommandTimeout @unit(s) = default(0.9s);
        double resendCommandTimeout @unit(s) = default(0.5s);
        int numServers = default(5);

        @display("i=block/source");
        @signal[commandResponseTime](type="simtime_t");
        @statistic[commandResponseTime](title="Total response time of client request"; unit=s; record=vector,stats,mean,max; interpolationmode=linear);

    gates:
        input in[];
        output out[];
}

simple StatsCollector
{
    parameters:
        int numServers = default(5);

        @display("p=20,60");
        @signal[consensusTime](type="simtime_t");
        @signal[consensusMessages](type="long");
        @statistic[consensusTime](title="Time required to reach consensus on a new leader, since network is leaderless"; unit=s; record=vector,mean,max,min; interpolationmode=linear);
        @statistic[consensusMessages](title="Number of messages exchanged on the network to reach consensus among servers"; record=vector,max,mean; interpolationmode=linear);
    gates:
        input in[];
}

simple DelayQueue
{
    parameters:
        volatile double avgServiceTime @unit(s) = default(0.02s);
        @display("i=block/queue");
    gates:
        input in;
        output out;
}

module ServerNode
{
    parameters:
        int numServers = default(5);
        int numClients = default(1);
    gates:
        input port_in[];
        input port_fromclients[];
        output port_out[];
        output port_toclients[];
        output port_toStatsCollector;
    submodules:
        server: Server {
            @display("p=180,142");
        }
        queue_servers[sizeof(port_out)]: DelayQueue {
            @display("p=72,213");
        }
        queue_clients[sizeof(port_toclients)]: DelayQueue {
            @display("p=272,226");
        }
    connections:
        // Server to StatsCollector
        server.toStatsCollector --> port_toStatsCollector;

        // Server to queues_servers
        for i=0..sizeof(port_out)-1 {
            server.out++ --> queue_servers[i].in;
        }

        // Server to queues_clients
        for i=0..sizeof(port_toclients)-1 {
            server.toclients++ --> queue_clients[i].in;
        }

        // Inputs received from others servers to inputs of the server
        for i=0..sizeof(port_in)-1 {
            port_in[i] --> server.in++;
        }

        // Inputs received from clients to inputs(fromclient) of the server
        for i=0..sizeof(port_fromclients)-1 {
            port_fromclients[i] --> server.fromclients++;
        }

        // Output for other servers, from the queue to the Node output
        for i=0..sizeof(port_out)-1 {
            queue_servers[i].out --> port_out[i];
        }

        // Output for clients, from the queue to the Node output
        for i=0..sizeof(port_toclients)-1 {
            queue_clients[i].out --> port_toclients[i];
        }
}

network Raft
{
    parameters:
        int numServers = default(5);
        int numClients = default(1);

        double channel_omission_probability = default(0.05);

        // The system should satisfies the following timing requirement:
		// broadcastTime << electionTimeout << MTBF
        double broadcastTime @unit(ms) = default(uniform(10ms, 20ms));
        double MTBF @unit(d) = default(uniform(100d, 200d));


    types:
        channel Channel extends ned.DelayChannel
        {
            delay = broadcastTime/2;
        }
        channel StatsCollectorChannel extends ned.DelayChannel
        {
            delay = 0;
            @display("ls=red,0");
        }

    submodules:
        server_node[numServers]: ServerNode {
            @display("p=95,37");
        }
        client[numClients]: Client {
            @display("p=174,76");
        }
        statsCollector: StatsCollector;
    connections:
        for i=0..numServers-1, for j=i+1..numServers-1 {
            server_node[i].port_out++ --> Channel --> server_node[j].port_in++;
            server_node[i].port_in++ <-- Channel <-- server_node[j].port_out++;
        }
        for i=0..numServers-1 {
            server_node[i].port_toStatsCollector --> StatsCollectorChannel --> statsCollector.in++;
        }

        for i=0..numClients-1, for j=0..numServers-1 {
            client[i].out++ --> Channel --> server_node[j].port_fromclients++;
            client[i].in++ <-- Channel <-- server_node[j].port_toclients++;
        }
}
