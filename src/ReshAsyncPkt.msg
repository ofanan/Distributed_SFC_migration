cplusplus {{
#pragma once
#include "Chain.h"
}}

class noncobject Chain; 

packet ReshAsyncPkt
{
	int deficitCpu;
	int reshInitiatorLvl;
  Chain pushDwnVec [];
}

