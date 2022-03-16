package src;

//
// TODO auto-generated type
//
module Datacenter
{
    parameters:
        int numChildren = default (2);
        @display("i=block/routing");
    gates:
        inout toParent;
        inout toChild[numChildren];
}
