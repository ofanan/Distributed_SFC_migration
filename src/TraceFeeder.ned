package src;

//
// Read the input files, showing users' mobility, and call the leaf datacenters to place new/migrated chains accordingly.
//

simple TraceFeeder
{
    parameters:
        @display("i=misc/node_vs,gold");
	    double decisionPariod @unit(s) = default(1s);
        
}