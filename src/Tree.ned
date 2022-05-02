// Generate Luembourg's service network
package src;

network Tree
{
  parameters:
  	string name     		= default("Tree");
		int height 					= default (3);
    int numChildren 		= default (4);
    int numDatacenters 	= default(int((numChildren^height-1)/(numChildren-1)));
		int numLeaves   		= default(int (numChildren^(height-1)));
    double channelDelay	@unit(s); 
    double basicDatarate @unit(Gbps);

  types:
    channel Channel extends ned.DatarateChannel {
      delay 	 = channelDelay;
      datarate = dataRate;
    }

  submodules:

		trace_feeder 	: TraceFeeder;

//		chain_generator : ChainGenerator;
		
//		cpu_augmenter 	: CpuAugmenter;

//		sol_checker		: SolChecker;
		
//		param_checker 	: ParamChecker;//

    datacenters [numDatacenters] : Datacenter {
        parameters:
          id = index;            
    };
  connections allowunconnected:

		// Connect the root. This is a special case, as the root has one less port (it doesn't haveu a parent). Furhtermore, in Monaco, the root has only 3 children.
		for child=1..numChildren { 
			datacenters[0].port[child-1] <--> {delay=channelDelay; datarate=basicDatarate;} <--> datacenters[child].port[0];		
		}

        for hop_cnt_from_root=2..height-1, for dc=int((numChildren^hop_cnt_from_root-1)/(numChildren-1))..int((numChildren^(hop_cnt_from_root+1)-1)/(numChildren-1)-1) {
            datacenters[dc].port[0] <--> {delay=channelDelay; datarate=basicDatarate*hop_cnt_from_root;} <--> datacenters[int((dc-1)/numChildren)].port[dc%numChildren+1];
        }
}

