cplusplus {{
#pragma once
#include "Chain.h"
}}

class noncobject Chain; 

packet ReshAsyncPkt
{
	int deficitCpu;
	int initiator;
  Chain pushDwnVec [];
}

