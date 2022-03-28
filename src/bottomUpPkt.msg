cplusplus {{
#pragma once
#include <list>
#include "Chain.h"
}}

class noncobject Chain; 
class noncobject ChainList; 

packet bottomUpPkt
{
    ChainList notAssigned;
    ChainList pushUpList;
}

