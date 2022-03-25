// Generate Luembourg's service network
package src;

network Lux
{
    parameters:
        int height 			= default (3);
        int numChildren 	= default (4);
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

		// Connect the root. This is a special case, as the root has one less port (it doesn't hae
		for child=1..numChildren { 
			datacenter[0].port[child-1] <--> {delay=channelDelay; datarate=basicDatarate;} <--> datacenter[child].port[0];		
		}

        for hop_cnt_from_root=2..height-1, for dc=int((numChildren^hop_cnt_from_root-1)/(numChildren-1))..int((numChildren^(hop_cnt_from_root+1)-1)/(numChildren-1)-1) {
            datacenter[dc].port[0] <--> {delay=channelDelay; datarate=basicDatarate*hop_cnt_from_root;} <--> datacenter[int((dc-1)/numChildren)].port[dc%numChildren+1];
        }

//		datacenter[0].port[0] <--> {delay=channelDelay; datarate=basicDatarate;} <--> datacenter[1].port[0];		
}


