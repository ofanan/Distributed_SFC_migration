// A message directly-sent from a datacenter, to the SimController, to inform about a chain's placement. 

cplusplus {{
#pragma once
}}

class noncobject Chain; 

message placementInfoMsg
{
  uint32_t newlyPlacedChains[]; // A list of chains that the sending datacenter has just placed.
}


