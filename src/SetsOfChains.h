#ifndef SETS_OF_CHAINS_H
#define SETS_OF_CHAINS_H

#include "Chain.h"

bool UsesMoreCpu (const Chain& lhs, const Chain& rhs);

using UsesMoreCpuType = std::integral_constant<decltype(&UsesMoreCpu), &UsesMoreCpu>;

typedef set<Chain, UsesMoreCpuType> SetOfChainsOrderedByDecCpuUsage;

#endif
