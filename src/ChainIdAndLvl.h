#ifndef CHAIN_ID_N_LVL
#define CHAIN_ID_N_LVL

#include "MyTypes.h"
#include "ChainsMaster.h"

using namespace std;

class ChainIdAndLvl {
	public:
	ChainId_t chainId;
	Lvl_t			lvl;
	
/*	ChainIdAndLvl ();*/
	ChainIdAndLvl (ChainId_t chainId=DUMMY_CHAIN_ID, Lvl_t lvl=UNPLACED_LVL);
	inline Cpu_t getCpu () const {return 7;} //{return ChainsMaster::getCpu (this->chainId, this->lvl);} //$$$
};

inline bool compareChainsIdsByDecCpuUsage (const ChainIdAndLvl &lhs, const ChainIdAndLvl &rhs) {
	Cpu_t lhsCpu = lhs.getCpu();
	Cpu_t rhsCpu = rhs.getCpu();
	if (lhsCpu==UNPLACED_CPU ||  rhsCpu==UNPLACED_CPU) {  // break ties
		return true;
	}
	return lhsCpu > rhsCpu;
}

void insertSorted (list <ChainIdAndLvl> &sortedList, const ChainIdAndLvl element);

#endif
