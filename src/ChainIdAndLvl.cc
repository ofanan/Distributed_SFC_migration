#include "ChainIdAndLvl.h"

ChainIdAndLvl::ChainIdAndLvl (ChainId_t chainId, Lvl_t lvl)
{
	chainId = chainId;
	lvl			= lvl;
}

//inline Cpu_t ChainIdAndLvl::getCpu () const
//{
//	return ChainsMaster::getCpu (this->chainId, this->lvl);
//}
