package src;

//
// Central controller of the simulation. Does the following:
// - Read the input files, showing users' mobility, and call the leaf datacenters to place new/migrated chains accordingly.
// - Collect and print stat about costs etc. 
//

simple SimController
{
    parameters:
        @display("i=misc/node_vs,gold");

//    private:
//        virtual void initialize();
//        virtual void handleMessage (cMessage *msg);
    
        
}