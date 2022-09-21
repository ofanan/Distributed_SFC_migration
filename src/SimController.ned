package src;

//
// Read the input files, showing users' mobility, and call the leaf datacenters to place new/migrated chains accordingly.
//

simple SimController
{
  parameters:
    @display("i=misc/node_vs,gold");
    double decisionPariod @unit(s) = default(1s);
	  int sizeofRtChain 	 = default (1);
    int sizeofNonRtChain = default (100);
gates:
		input directMsgsPort;
}
