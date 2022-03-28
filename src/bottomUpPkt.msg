cplusplus {{
#pragma once
#include "Chain.h"
}}

class noncobject Chain; 
class noncobject ChainList; 

packet bottomUpPkt
{
    ChainList notAssigned;
    ChainList pushUpList;
}

