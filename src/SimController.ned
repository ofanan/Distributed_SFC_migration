package src;

//
// Read the input files, showing users' mobility, and call the leaf datacenters to place new/migrated chains accordingly.
//

simple SimController
{
  parameters:
    @display("i=misc/node_vs,gold");
	  int sizeofRtChain 	 = default (1);
    int sizeofNonRtChain = default (100);
    int cpuAtLeaf				 = default (1);
    int    seed					 = default (42);
    double simLenInSec   = default (999999); // (numeric_limits<float>::max());
    bool   syncMode			 = default (false);
    double RtProb				 = default (0.3); // prob' that a new chain is an RT chain
    bool logDelays			 = default (false);
		bool manuallySetPktSize = default (true);
    bool   runningRtProbSim = default (false);
    double decisionPariod @unit(s) = default(1s);
    int byteLengthOfHeader 	= default (4);
    int byteLengthOfRtChain = default (10);
    int byteLengthOfNonRtChain = default (15);
gates:
		input directMsgsPort;
}
