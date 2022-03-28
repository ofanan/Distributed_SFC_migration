cplusplus {{
#pragma once
#include "Chain.h"
}}

//class noncobject Chain; 
//class noncobject ChainList; 

packet bottomUpPkt
{
    int route[];
//    Chain notAssigned[];
//    ChainList notAssigned; // Should fix by 6.2.8 in the RTFM.
//    ChainList pushUpList;
}

