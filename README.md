# Distributed_SFC_migration

This project implements a distributed asynchronous protocol for the deployment and migration of Micro-Services (MSs) in the edge-cloud continuum. 
The protocol is implemented using the Omnet++ simulator, and is based on real-world antenna location, and on SUMO (Simulator of Urban MObility).

Further details can be found in the following paper: 

I. Cohen, P. Giaccone, and C.F. Chiasserini, [Distributed Asynchronous Protocol for Service Provisioning in the Edge-Cloud Continuum](https://www.researchgate.net/profile/Itamar-Cohen-2/publication/371722549_Distributed_Asynchronous_Protocol_for_Service_Provisioning_in_the_Edge-Cloud_Continuum/links/6491d2a5c41fb852dd1b1c79/Distributed-Asynchronous-Protocol-for-Service-Provisioning-in-the-Edge-Cloud-Continuum.pdf), IEEE International Conference on Software, Telecommunications and Computer Networks (SoftCom), 2023.

### Directories
Omnet++'s code files (.cc, .h, .msg, .ned) are found in ./src. 
The locations of vehicles along the simulated trace are found in ./res/pos_files.
.poa (Point Of Access) files detail the location of each vehicle along the trace.
Further details on how to generate and use .poa files can be found in the [SFC_migration rep](https://github.com/ofanan/SFC_migration).

The directory ./ contains Omnet++'s configuration files (.ini), running scripts, and the results .comoh files.

### .ini files
.ini files detail the network's Omnet configuration. 
The files Lux.ini, Monaco.ini detail the topology of Luxembourg and Monaco's networks.
The files UniformTree.ini, NonUniformTree.ini detail the topology of smaller networks, used for development and debugging.
Common.ini contains parameters that are common to Lux and Monaco.

### source files
Below is an overview of Omnet++'s code files, which are located in ./src. 
##### SimController.cc 
The main simulator's file. The file initializes the netowk and runs a simulation according to the chosen .ini file.
##### Datacenter.cc 
The implementation of a single datacenter.
##### Chain.cc 
The data of a single VM, e.g.: the required capacity and cost of deploying it in each level.
##### ChainMaster.cc
Class for handling lists of chains.
##### MyConfig.cc
Configuration parameters, costs, and some accessory functions.

### Output files
The simulations' output are written to files in the project's main directory.
.comoh files are files that detail the communication overhead along the trace.
.res files detail, for each second in the simulated trace, whether a feasible sol was found, and if so - it costs computation, link and migration costs.
.log files are used mainly for development / debugging.
