#ifndef CHAIN_H
#define CHAIN_H
#include <stdio.h>
#include <stdint.h>
#include <omnetpp.h>

#include <vector>
#include <type_traits>
#include <set>
#include <algorithm>
#include <unordered_set>
using namespace std;

#include "MyTypes.h"
#include "MyConfig.h"


class Chain
{

  public:
    ChainId_t id;
    vector <DcId_t> S_u;         // List of delay-feasible datacenters for this chain
    bool isRtChain;
		const static vector<Cost_t> costOfCpuUnitAtLvl; 
		Lvl_t  curLvl;        // Level of the datacenter currently assigned to host me. This may change a few times until the real placement as curDatacenter
		DcId_t curDc; 
		Cpu_t  potCpu; // the cpu that this chain needs, if placed on its currently pot-placed placed

		// C'tors
    Chain ();
		Chain (const Chain &c);
    Chain (ChainId_t id);
    Chain (ChainId_t id, vector <DcId_t> &S_u, Lvl_t curLvl=UNPLACED_LVL);
            
    bool operator== (const Chain &right) const { //$$$$
      return (int(this->id) == int(right.id));
    }
	
		void print (bool printS_u = true);	
	
		// Getters
    Cost_t getCost 		() const; // non-mig' cost of the chain at its current place
    Cost_t getCostAtLvl (const Lvl_t lvl) const; // non-mig' cost of the chain if placed in a certain given level
    Cpu_t  getCpu     () const;     
    Cpu_t mu_u_at_lvl (Lvl_t lvl) const; // returns the amount of cpu required for placing this chain at level lvl
    Lvl_t mu_u_len () const;
		bool dcIsDelayFeasible (DcId_t dcId, Lvl_t dcLvl) {return this->S_u[dcLvl]==dcId;}

		// Setters
		void setPotCpu (Lvl_t lvl); // set this->potCpu according to the lvl of the pot-placing host, and the characteristic of the chain
};

class RtChain : public Chain
{
public:
  static vector<Cpu_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
  static Lvl_t mu_u_len;
	static vector<Cost_t> costAtLvl; // cpuCostAtLvl[i] will hold the cost of placing an RT chain on a DC in level i
	RtChain (const RtChain &c);
  RtChain (ChainId_t id, vector <DcId_t> &S_u);
};

class NonRtChain: public Chain
{
  public:
	  static vector<Cpu_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
	  static Lvl_t  mu_u_len;
		static vector<Cost_t> costAtLvl; // cpuCostAtLvl[i] will hold the cost of placing an RT chain on a DC in level i
    NonRtChain (ChainId_t id, vector <DcId_t> &S_u);
	  NonRtChain (const NonRtChain &c);
};

/*************************************************************************************************************************************************
Sort the chains for notAssignedList, as follow:
- give higher priority to RT chains.
- break ties by an inc. order of id.
**************************************************************************************************************************************************/
struct SortChainsForNotAssignedList {
	size_t operator()(const Chain& lhs, const Chain&rhs) const {
		if (lhs.isRtChain && (!rhs.isRtChain)) { // give higher priority to RT chains
			return true;
		}
		if (!lhs.isRtChain && (rhs.isRtChain)) { // give higher priority to RT chains
			return false;
		}		
			return (lhs.id < rhs.id);
	} 
};


/*************************************************************************************************************************************************
Sort the chains for pushUpList, as follow:
- sort by decreasing order of the # of CPU units each chain is currently using.
- Break ties by how much CPU each of them will be using if pushed-up to a certain level; usrs that would consume lower cpu when pushed-up, should appear first.
  This is approximated merely by letting non-RT chains appear first, because for a given level, 
  non-RT chains are likely to consume less cpu than the RT equivalents.
- Break further ties by an increasing id order.
**************************************************************************************************************************************************/
struct SortChainsForPushUpList {
	size_t operator()(const Chain& lhs, const Chain&rhs) const {
	if (lhs.potCpu > rhs.potCpu) {
		return true;
	}
	if (lhs.potCpu < rhs.potCpu) {
		return false;
	}
	if (!lhs.isRtChain && rhs.isRtChain) {
		return true;
	}
	if (lhs.isRtChain && !rhs.isRtChain) {
		return false;
	}
	return (lhs.id < rhs.id);
  }
};

// Instruct the compiler to identify (and, in particular, hash) Chains based on theirs id only.
struct ChainHash {
	size_t operator()(const Chain& c) const {
  	return hash<ChainId_t>()(c.id);
  }
};

typedef unordered_set <Chain, ChainHash> UnorderedSetOfChains;

/*************************************************************************************************************************************************
Rcvs 2 sorted vectors of chains. 
Put in the first vector (given by ref') a sorted vector, containing the union of the two input vectors. 
**************************************************************************************************************************************************/
void MergeSort (vector <Chain> &vec, const vector <Chain> vec2union);

// Insert a chain in its correct place to a sorted datastructure
void insertSorted (vector <Chain> &vec, const Chain &c); // Insert a chain c to the correct place in the vector, based on its latency tightness.
inline bool CompareChainsByDecCpuUsage (const Chain & lhs, const Chain & rhs);
bool insertChainToList (list <Chain> &sortedList, const Chain &c); // Insert a chain c to the correct place in the vector, based on its latency tightness.

/*************************************************************************************************************************************************
// Given chainId, assigns to chain the respective chain from the set. returns true iff the requested chain Id was found in the set.
**************************************************************************************************************************************************/
bool findChainInSet   (const UnorderedSetOfChains setOfChains , ChainId_t chainId, Chain &c); 
bool findChainInSet 	(const set<Chain> setOfChains, ChainId_t id, Chain& foundChain); 
/*bool findChainInSet (unordered_set <Chain, ChainHash> setOfChains, ChainId_t chainId, Chain &c)*/
bool eraseChainFromSet (UnorderedSetOfChains &setOfChains, ChainId_t chainId); // Given chainId, erases the respective chain from the set. 

vector<Chain> findChainsByPoa (UnorderedSetOfChains setOfChains, DcId_t poa);

#endif



