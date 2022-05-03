// A message directly-sent from the traceFeeder datacenters, to inform it that a chain that it hosted left, and the datacenter should release its resources.

cplusplus {{
#pragma once
#include "Chain.h"
}}

class noncobject Chain; 

message leftChainsMsg
{
	int32_t leftChains[];
}


