#include "ChainIdAndLvl.h"

ChainIdAndLvl::ChainIdAndLvl (ChainId_t chainId, Lvl_t lvl)
{
	chainId = chainId;
	lvl			= lvl;
}

/*************************************************************************************************************************************************
insert a ChainIdAndLvl to its correct location in a sorted list.
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

