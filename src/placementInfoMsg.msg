// A message directly-sent from a datacenter, to the traceFeeder, to inform about a chain's placement. 

cplusplus {{
#pragma once
}}

class noncobject Chain; 

message placementInfoMsg
{
  int32_t placedChains[]; // A list of chains that the sending datacenter has just placed.
}


