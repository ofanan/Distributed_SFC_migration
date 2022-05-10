cplusplus {{
#pragma once
#include "Chain.h"
}}

class noncobject Chain; 
//class noncobject ChainList; 

packet bottomUpPkt
{
    Chain notAssigned[];
    Chain pushUpVec  [];
}

