package src;
//import node.*;

network Tree
{
    parameters:
        int height 						= default (3);
        int numChildren 				= default (3);
        int numDatacenters 				= int((numChildren^height-1)/(numChildren-1));
//        int idxOfLastNonLeaf 			= int(numNodes-numChildren^height-1);
//        int	smallestLeafIdx 			= idxOfLastNonLeaf+1;
        datacenters[0].numParents = 0;
//        int numChildrenVec		= 
        double channelDelay	@unit(s); 
        double basicDatarate @unit(Gbps);

        types:
        channel Channel extends ned.DatarateChannel {
            delay 	 = channelDelay;
            datarate = dataRate;
        }

    submodules:

		tracefeeder : TraceFeeder;

		chainggenerator : ChainGenerator;

		solchecker: SolChecker;

        datacenters [numDatacenters] : Datacenter {
            parameters:
            id = index;
        };
            
    connections allowunconnected:
        for h=1..height-1, for c=int((numChildren^h-1)/(numChildren-1))..int((numChildren^(h+1)-1)/(numChildren-1)-1) {
            datacenters[c].toParent[0] <--> {delay=channelDelay; datarate=basicDatarate*h;} <--> datacenters[int((c-1)/numChildren)].toChild++;
        }
}
