package src;

//
// TODO auto-generated type
//
network Node
{
    parameters:
        int numChildren; // = default (2);
        int numParents    = default (1);
        bool isRoot 	  = default (false);
        @display("i=misc/node_vs,gold");
        
    gates:
        inout portToParent[numParents];
        inout portToChild[numChildren];
         
    submodules:
        datacenter: Datacenter {
            parameters:
                @display("p=140,130");
				        numChildren = numChildren;
				        numParents	= numParents; 
        }        
//        queue[numChildren+numParents]: L2Queue {
//            parameters:
//                @display("p=80,200,row");
//        }

        connections allowunconnected:
//        datacenter.toParent[0] <--> queue[0].local  if !isRoot;
//        queue[0].line		   <--> portToParent[0] if !isRoot;
//        datacenter.toChild[0]  <--> queue[1].local;
//        for c=0..numChildren-1 {
//            datacenter.toChild[c] <--> queue[c+numParents].local if numChildren>0;
//            queue[c+1-int(isRoot)].line 		  <--> portToChild[c] if numChildren>0;   
//        }
}
