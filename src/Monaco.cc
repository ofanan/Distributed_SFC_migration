//Write here all the shortest paths of Monaco's leaves to the root?
//This won't work for a full tree...
//But for a full tree we can know this using comb' considerations.

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <set>

using namespace omnetpp;
using namespace std;

class Monaco : public cSimpleModule
{
	public:
	int16_t pathToRoot[2][3] = {{2,3,4}, {3,3,4}};
};

