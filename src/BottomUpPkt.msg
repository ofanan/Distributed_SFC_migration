cplusplus {{
#pragma once
#include "Chain.h"
}}

class noncobject Chain; 
//class noncobject ChainList; 

packet BottomUpPkt
{
	Chain notAssigned[];
	ChainIdAndLvl pushUpVec  [];
}

