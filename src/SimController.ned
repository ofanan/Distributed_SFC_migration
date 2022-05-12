package src;

//
// Read the input files, showing users' mobility, and call the leaf datacenters to place new/migrated chains accordingly.
//

simple SimController
{
  parameters:
    @display("i=misc/node_vs,gold");
    double RT_prob = 0; // prob' that a new chain is an RT chain
    double decisionPariod @unit(s) = default(1s);
//	port:
//		input directMsgsPort;
}
