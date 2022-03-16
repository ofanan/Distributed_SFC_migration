package src;

//
// TODO auto-generated type
//
module Datacenter
{
    parameters:
        int numChildren;
        int numParents;
        @display("i=block/routing");
    gates:
        inout toParent[numParents];
        inout toChild [numChildren];
}
