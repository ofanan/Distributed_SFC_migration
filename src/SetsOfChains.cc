#include "SetsOfChains.h"

bool UsesMoreCpu (const Chain& lhs, const Chain& rhs)
{
	int16_t lhsCpu = lhs.getCpu ();
	int16_t rhsCpu = rhs.getCpu ();
	if (lhsCpu == rhsCpu) {  // break ties
		return (lhs.id > rhs.id);
	}
	return lhsCpu > rhsCpu;
}

