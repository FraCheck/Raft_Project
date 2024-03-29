package raft_project.simulations;

@license(LGPL);

simple Server
{
    parameters:
        volatile double electionTimeout @unit(s) = default(uniform(0.150s,0.300s));
        volatile double heartbeatPeriod @unit(s) = default(uniform(0.100s,0.150s));
        double server_failure_probability = default(0.2);
        double leader_failure_Probability = default(0.2);
        bool canFail = default(false);
        volatile double crashTimeoutAvg @unit(s) = default(1s);
        double recoverTimeoutAvg @unit(s) = default(0.5s);
        bool test= default(false);
        string test_type = default("no_test");

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

    gates:
        input in[];
        output out[];
        output toStatsCollector;
}

simple StatsCollector
{
    parameters:

        @display("p=20,60");
        @signal[consensusTime](type="simtime_t");
        @signal[consensusMessages](type="long");
        @signal[timeToRecoverLog](type="simtime_t");
        @signal[commitMessages](type="long");
        @signal[commandResponseTime](type="simtime_t");
        @statistic[consensusTime](title="Time required to reach consensus on a new leader, since network is leaderless"; unit=s; record=vector,mean,max,min,; interpolationmode=linear);
        @statistic[consensusMessages](title="Number of messages exchanged on the network to reach consensus among servers"; record=vector,max,mean,min; interpolationmode=sample-hold);
        @statistic[timeToRecoverLog](title="Time required for a server to recover its log to the most up to date version possible"; unit=s; record=vector,mean,max,min; interpolationmode=linear);
        @statistic[commitMessages](title="Number of messages exchanged on the network to commit a command in the log"; record=vector,max,mean,min; interpolationmode=sample-hold);
        @statistic[commandResponseTime](title="Total response time of clients requests"; unit=s; record=vector,stats,mean,max,min; interpolationmode=none);
        
    gates:
        input in[];
        input in_clients[];

}

simple DelayQueue
{
    parameters:
        volatile double avgServiceTime @unit(s) = default(0.001s);
        @display("i=block/queue");
    gates:
        input in;
        output out;
}
simple Mock{
    parameters:
        string test_type;
    gates:
    input in;
    input fromserver[];
    input fromstats;
    output out;
}

module ServerNode
{
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

        // Output for clients, from the queue to the ServerNode output
        for i=0..sizeof(port_toclients)-1 {
            queue_clients[i].out --> port_toclients[i];
        }
}

module ClientNode
{
    parameters:
        @display("i=block/source");

    gates:
        input port_in[];
        output port_out[];
        output port_toStatsCollector;

        // To implement in case of new statistics that involve the StatsCollector
        //output port_toStatsCollector;

    submodules:
        client: Client {
            @display("p=180,142");
        }
        queue_servers[sizeof(port_out)]: DelayQueue {
            @display("p=72,213");
        }

    connections:
        // To implement in case of new statistics that involve the StatsCollector
        // Client to StatsCollector
        client.toStatsCollector --> port_toStatsCollector;

        // Client to queues_servers
        for i=0..sizeof(port_out)-1 {
            client.out++ --> queue_servers[i].in;
        }

        // Inputs received from servers to inputs of the client
        for i=0..sizeof(port_in)-1 {
            port_in[i] --> client.in++;
        }

        // Output for servers, from the queue to the ClientNode output
        for i=0..sizeof(port_out)-1 {
            queue_servers[i].out --> port_out[i];
        }
}

network Raft
{
    parameters:
        int numServers = default(5);
        int numClients = default(1);

        double channel_omission_probability = default(0.01);
        bool networkDelaysPresent = default(true);
        bool displayLogs = default(true);
        bool disableStatsCollector = default(false);

        // The system should satisfies the following timing requirement:
		// broadcastTime << electionTimeout << MTBF

        //The broadcast time should be an order of magnitude less than the
        // election timeout so that leaders can reliably send the heartbeat messages
        //required to keep followers from starting elections
        double broadcastTime @unit(s) = default(uniform(0.010s, 0.020s));
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
        client_node[numClients]: ClientNode {
           // @display("p=174,76");
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

        for i=0..numClients-1 {
            client_node[i].port_toStatsCollector --> StatsCollectorChannel --> statsCollector.in_clients++;
        }

        for i=0..numClients-1, for j=0..numServers-1 {
            client_node[i].port_out++ --> Channel --> server_node[j].port_fromclients++;
            client_node[i].port_in++ <-- Channel <-- server_node[j].port_toclients++;
        }
}
network Test
{

    submodules:
        server: Server {
            parameters:
                @display("i=,cyan"); // do not change the icon (first arg of i=) just colorize it

        }
        mock: Mock {
            parameters:
                @display("i=,gold"); // here too
        }
    connections:
        server.out++  --> {} -->  mock.in;
        server.toclients++ --> {} -->  mock.fromserver++;
        server.toStatsCollector --> {delay = 100ms;} -->  mock.fromstats;
        server.in++ <-- {delay = 100ms;} <-- mock.out;

}
