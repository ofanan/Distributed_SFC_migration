cplusplus {{
#pragma once
#include "Chain.h"
#include "MyTypes.h"
#include "ChainIdAndLvl.h"
}}

class noncobject Chain; 
class noncobject ChainIdAndLvl; 

packet PushUpPkt
{
	ChainIdAndLvl pushUpVec [];
}

