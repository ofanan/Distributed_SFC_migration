#ifndef SETS_OF_CHAINS_H
#define SETS_OF_CHAINS_H

#include <stdio.h>
#include <stdint.h>
#include <omnetpp.h>

#include <vector>
#include <set>
#include <algorithm>
#include <unordered_set>
#include "Chain.h"

using namespace std;
class Chain;

bool UsesMoreCpu (const Chain& lhs, const Chain& rhs);

using UsesMoreCpuType = std::integral_constant<decltype(&UsesMoreCpu), &UsesMoreCpu>;

typedef set<Chain, UsesMoreCpuType> SetOfChainsOrderedByDecCpuUsage;

#endif
