// A message directly-sent from the traceFeeder to leaf datacenters, to initiate a run of our alg'.

cplusplus {{
#pragma once
#include "Chain.h"
}}

class noncobject Chain; 
//class noncobject ChainList; 

message initBottomUpMsg
{
    Chain notAssigned[];
    Chain pushUpList [];
//    ChainList notAssigned; // Should fix by 6.2.8 in the RTFM.
}

