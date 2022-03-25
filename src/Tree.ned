package src;

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

//		trace_feeder 	: TraceFeeder;
//
//		chain_generator : ChainGenerator;
//		
//		cpu_augmenter 	: CpuAugmenter;
//
//		sol_checker		: SolChecker;
//		
//		param_checker 	: ParamChecker;

        datacenter [numDatacenters] : Datacenter {
            parameters:
        };
            
    connections allowunconnected:
//        for h=1..height-1, for c=int((numChildren^h-1)/(numChildren-1))..int((numChildren^(h+1)-1)/(numChildren-1)-1) {
//            datacenter[c].toParent[0] <--> {delay=channelDelay; datarate=basicDatarate*h;} <--> datacenter[int((c-1)/numChildren)].toChild++;
//        }
}
