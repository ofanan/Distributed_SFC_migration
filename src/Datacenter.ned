package src;

//
// TODO auto-generated type
//
simple Datacenter
{
    parameters:
        int numChildren;
        int numParents;
        bool isLeaf = (numChildren==0);
        @display("i=block/routing");
        
        // Add required params for the Alg, e.g.:
        // CPU cap'.
        // Have trace-reader, and, at a different module, collect stat and costs.
        // Have a module responsible for the rsrc aug. Read in the documnetation about global vars.
        // Beware that some STL (datastructrues of Cpp (arrays, vectors?)) may be not compatible with Omnetpp. Then, read the documentation to find an omnetpp compatible equivalent.
        // variables: avail cpu, assigned chains, ...
    gates:
        inout toParent[numParents];
        inout toChild [numChildren];
}
