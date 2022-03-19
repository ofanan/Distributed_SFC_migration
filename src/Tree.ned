package src;
//import node.*;

network Tree
{
    parameters:
        int height 			= default (3);
        int numChildren 	= default (3);
        int numDatacenters 	= default(int((numChildren^height-1)/(numChildren-1)));
		int numLeaves   	= default(int (numChildren^(height-1)));
        double channelDelay	@unit(s); 
        double basicDatarate @unit(Gbps);

        types:
        channel Channel extends ned.DatarateChannel {
            delay 	 = channelDelay;
            datarate = dataRate;
        }

    submodules:

		tracefeeder 	: TraceFeeder;

		chainggenerator : ChainGenerator;
		
		cpuaugmenter 	: CpuAugmenter;

		solchecker		: SolChecker;
		
		paramchecker 	: ParamChecker;

        datacenters [numDatacenters] : Datacenter {
            parameters:
        };
            
    connections allowunconnected:
        for h=1..height-1, for c=int((numChildren^h-1)/(numChildren-1))..int((numChildren^(h+1)-1)/(numChildren-1)-1) {
            datacenters[c].toParent[0] <--> {delay=channelDelay; datarate=basicDatarate*h;} <--> datacenters[int((c-1)/numChildren)].toChild++;
        }
}
