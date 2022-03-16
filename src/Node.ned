package src;

//
// TODO auto-generated type
//
network Node
{
    parameters:
        int numChildren; // = default (2);
        bool isRoot 	  = default (false);
        @display("i=misc/node_vs,gold");
        
    gates:
        inout portToParent;
        inout portToChild[numChildren];
         
    submodules:
        datacenter: Datacenter {
            parameters:
                @display("p=140,130");
				        numChildren 	= numChildren; 
        }        
        queue[numChildren+1-int(isRoot)]: L2Queue {
            parameters:
                @display("p=80,200,row");
        }

        connections allowunconnected:
        datacenter.toParent$o --> queue[0].local$i if !isRoot;
        datacenter.toParent$i <-- queue[0].local$o if !isRoot;
        queue[0].line		    <--> portToParent	     if !isRoot;
        for c=0..numChildren-1 {
            datacenter.toChild[c] <--> queue[c+1-int(isRoot)].local if numChildren>0;
            queue[c+1-int(isRoot)].line 		  <--> portToChild[c] if numChildren>0;   
        }
}
