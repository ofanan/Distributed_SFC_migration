package src;
//import node.*;

network Tree
{
    parameters:
        int height 				= default (2);
        int numChildren 		= default (3);
        int numNodes 			= int((numChildren^height-1)/(numChildren-1));
        int idxOfLastNonLeaf 	= int(numNodes-numChildren^height-1);
        int	smallestLeafIdx 	= int(numNodes-numChildren^height);
        node[0].isRoot 			= true;
        node[0..3].numChildren = numChildren; //default(50);
        node[4..6].numChildren = 0; //default(50);
//        int numChildrenVec		= 
//        int[2] gamad 			= [1, 0];
//        node[0..numNodes].numChildren 	= numChildren; 
//        node[0..(numNodes-numChildren^height-1)].numChildren 	= numChildren;

        types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }

    submodules:
            node [numNodes] : Node;
//            {
//                numChildren = numChildren; // if (index<smallestLeafIdx);
//    			gatesizes if index=0 : portToChild[0];
//          gatesizes if index==0 || index==count-1:
//    	in[1], out[1];
//          gatesizes:
//    	in[2], out[2];
//                    	parameters:
//            	    isRoot 		= false;
//            }
            
            
//    connections allowunconnected:
//        for h=1..height-1, for c=int((numChildren^h-1)/(numChildren-1))..int((numChildren^(h+1)-1)/(numChildren-1)-1) {
//            node[c].portToParent <--> Channel <--> node[int((c-1)/numChildren)].portToChild++;
//        }
        
}
