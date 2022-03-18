// Generate Luembourg's service network
package src;

network Lux
{
    types:
        channel Channel extends ned.DatarateChannel {
            delay 	 = channelDelay;
            datarate = dataRate;
        }

        submodules:
            datacenter [numDatacenters] : Datacenter {
                parameters:
                id = index;
            };
    connections allowunconnected:
        datacenter[0].toChild[0] <--> Channel <--> datacenter[1].toChild[0];
}


