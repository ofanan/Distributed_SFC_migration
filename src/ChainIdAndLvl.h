#ifndef CHAIN_ID_N_LVL
#define CHAIN_ID_N_LVL

#include "MyTypes.h"
#include "ChainsMaster.h"

class ChainIdAndLvl {
	public:
	ChainId_t chainId;
	Lvl_t			lvl;
	
	ChainIdAndLvl (ChainId_t chainId, Lvl_t lvl);
	inline Cpu_t getCpu () const {return ChainsMaster::getCpu (this->chainId, this->lvl);}
};

inline bool compareChainsIdsByDecCpuUsage (const ChainIdAndLvl &lhs, const ChainIdAndLvl &rhs) {
	Cpu_t lhsCpu = lhs.getCpu();
	Cpu_t rhsCpu = rhs.getCpu();
	if (lhsCpu==UNPLACED_CPU ||  rhsCpu==UNPLACED_CPU) {  // break ties
		return true;
	}
	return lhsCpu > rhsCpu;
}

/*************************************************************************************************************************************************
insert a chain to its correct location in a sorted list.
If the chainId is already found in the list, the old occurance in the list is deleted.
**************************************************************************************************************************************************/
void insertSorted (list <ChainIdAndLvl> &sortedList, const ChainIdAndLvl element)
{

	for (auto it = sortedList.begin(); it!=sortedList.end(); ) {
		if (it->chainId == element.chainId) {
			sortedList.erase (it);
			break;
		}
		it++;
	}

	auto begin 	= sortedList.begin();
  auto end 		= sortedList.end();
  while ((begin != end) && compareChainsIdsByDecCpuUsage (*begin, element)) { // skip all chains in the list which use more cpu than me
  	begin++;
  }
  sortedList.insert(begin, element);
}

#endif
