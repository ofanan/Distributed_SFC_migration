package src;

simple Datacenter
{
  parameters:
    @display("i=block/routing");
    int  dcId 			= default (0);
    int numParents 	= default (1);
    int numChildren = default (4); 
    int lvl; // level of this datacenter in the tree
    bool isRoot         = (numParents==0);
    bool isLeaf         = (numChildren==0);
    // Add required params for the Alg, e.g.:
    // CPU cap'.
    // Have trace-reader, and, at a different module, collect stat and costs.
    // Have a module responsible for the rsrc aug. Read in the documnetation about global vars.
    // Beware that some STL (datastructrues of Cpp (arrays, vectors?)) may be not compatible with Omnetpp. Then, read the documentation to find an omnetpp compatible equivalent.
      // variables: avail cpu, assigned chains, ...
  gates:
	inout port[numParents+numChildren];    
	input directMsgsPort;
}

