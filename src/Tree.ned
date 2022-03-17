package src;
//import node.*;

network Tree
{
    parameters:
        int height 						= default (3);
        int numChildren 				= default (3);
        int numNodes 					= int((numChildren^height-1)/(numChildren-1));
        int idxOfLastNonLeaf 			= int(numNodes-numChildren^height-1);
        int	smallestLeafIdx 			= idxOfLastNonLeaf+1;
        node[0].isRoot 					= true;
//        int numChildrenVec		= 
        double channelDelay	@unit(s); 
        double basicDatarate @unit(Gbps);

        types:
        channel Channel extends ned.DatarateChannel {
            delay 	 = channelDelay;
            datarate = dataRate;
        }

    submodules:
            node [numNodes] : Node {nodeId = index;};
            
    connections allowunconnected:
        for h=1..height-1, for c=int((numChildren^h-1)/(numChildren-1))..int((numChildren^(h+1)-1)/(numChildren-1)-1) {
//            node[c].portToParent[0] <--> Channel <--> node[int((c-1)/numChildren)].portToChild++;
            node[c].portToParent[0] <--> {delay=channelDelay; datarate=basicDatarate*h;} <--> node[int((c-1)/numChildren)].portToChild++;
        }
//        for i=0..idxOfLastNonLeaf, for c=0..numChildren-1 {
//            node[i].portToChild[c] <--> Channel <--> node[i*numChildren+c+1].portToParent[0];
//        }        
}
