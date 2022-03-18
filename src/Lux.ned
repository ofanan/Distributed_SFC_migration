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
            node [numNodes] : Node {
                parameters:
                nodeId = index;
            };
    connections allowunconnected:
        node[0].portToChild[0] <--> Channel <--> node[1].portToChild[0];
}


