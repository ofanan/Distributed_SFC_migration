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
    bool logComOh			   = default (false);
		bool manuallySetPktSize = default (true);
    bool   runningRtProbSim = default (false);
    double decisionPariod @unit(s) = default(1s);
    bool allowBlkChain = default (false); // allow blocking of new requests
    int bitLenOfHdr 	= default (10);
    double BU_ACCUM_DELAY_OF_LVL1;
    double RESH_ACCUM_DELAY_OF_LVL1;
		double beginVeryDetailedLogAtTraceTime;
		int    LOG_LVL = default (0);
		int    RES_LVL = default (1);
		int    DEBUG_LVL = default (1);
gates:
		input directMsgsPort;
}


