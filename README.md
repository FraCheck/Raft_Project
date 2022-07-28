# Raft algorithm implementation and simulation in Omnet++

Raft algorithm implementation and simulation in Omnet++ project for the Distributed Systems course at Politecnico di Milano (2021/2022).
The project has been realized by [Francesco Cecchetti](https://github.com/FraCheck), [Matteo Merz](https://github.com/mtt-merz) and [Fabio Patella](https://github.com/FabioPatella).

## What is the Raft consensus algorithm?
In a distributed systems setting, consensus algorithms allow a collection of machines to work as a coherent group that can survive the failures of some of its members.
From the [original paper](https://raft.github.io/raft.pdf) abstract:

Raft is a consensus algorithm for managing a replicated log. It produces a result equivalent to (multi-)Paxos, and it is as efficient as Paxos, but its structure is different from Paxos; this makes Raft more understandable than Paxos and also provides a better foundation for building practical systems. In order to enhance understandability, Raft separates the key elements of consensus, such as leader election, log replication, and safety, and it enforces
a stronger degree of coherency to reduce the number of states that must be considered. Results from a user study demonstrate that Raft is easier for students to learn than Paxos. Raft also includes a new mechanism for changing the cluster membership, which uses overlapping majorities to guarantee safety.

## Requirements of the project
Many assumptions are explained in the paper, the aim is to make the system robust to the following non-byzantine failures:
+ links can have omissions
+ links can have delays
+ processes can stop at any time

More hints and details about the implementation, simulation and statistics gathering, are present in the [Presentation.pdf](https://github.com/FraCheck/Raft_Project/blob/main/Deliverables/Presentation.pdf).


## A brief Demo GIF

![Demo](https://github.com/FraCheck/Raft_Project/blob/main/Deliverables/demo.gif)
