// A message directly-sent from the sim_controller to leaf datacenters, to initiate a run of our alg'.

cplusplus {{
#pragma once
#include "Chain.h"
}}

class noncobject Chain; 
//class noncobject ChainList; 

message InitBottomUpMsg
{
	Chain notAssigned[]; // new or critical chains to be assigned 
}


