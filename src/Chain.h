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

#include "MyTypes.h"
#include "MyConfig.h"

using namespace std;

class Chain
{
  public:
    ChainId_t id;
    vector <DcId_t> S_u;         // List of delay-feasible datacenters for this chain
    bool isRT_Chain;
		const static vector<Cost_t> costOfCpuUnitAtLvl; 
		Lvl_t curLvl;        // Level of the datacenter currently assigned to host me. This may change a few times until the real placement as curDatacenter
		DcId_t curDc; // 

		// C'tors
    Chain ();
		Chain (const Chain &c);
    Chain (ChainId_t id);
    Chain (ChainId_t id, vector <DcId_t> &S_u, Lvl_t curLvl=UNPLACED_LVL);
            
    bool operator== (const Chain &right) const {
      return (this->id == right.id);
    }
	
		void print (bool printS_u = true);	
	
		// Getters
/*		DcId_t getCurDatacenter () const; // returns the id of the datacenter currently hosting this; or UNPLACED_DC, if this chain isn't placed*/
/*		void setDc (DcId_t dcId) {this->id = dcId;};*/
    Cost_t getCpuCost () const;
    Cost_t getCost 		() const;
    Cpu_t  getCpu     () const; 
    
		/* 
		We order chain by non-increasing order of |S_u|, namely how high they can be located in the tree; iteratively breaking ties by decreasing mu_u[l] for each level \ell, namely, the amount of CPU 
		rsrcs required for locating the chain at a certain lvl.
		However, in our current sim settings, all of this is degenerated to the fast and efficient rule that an RT chain has higher priority (==smaller #), over a non-RT chain.
    bool operator< (const Chain &right) const {
      return (this->isRT_Chain && !(right.isRT_Chain));
    }
		*/		
    Cpu_t mu_u_at_lvl (Lvl_t lvl) const; // returns the amount of cpu required for placing this chain at level lvl
    Lvl_t mu_u_len () const;
    void  setS_u (const vector <DcId_t> &S_u);
		bool dcIsDelayFeasible (DcId_t dcId, Lvl_t dcLvl) {return this->S_u[dcLvl]==dcId;}
};

class RT_Chain : public Chain
{
public:
  static const vector<Cpu_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
  static const Lvl_t mu_u_len;
	static const vector<Cost_t> cpuCostAtLvl; // cpuCostAtLvl[i] will hold the cost of placing an RT chain on a DC in level i
	static const vector<Cost_t> costAtLvl; // cpuCostAtLvl[i] will hold the cost of placing an RT chain on a DC in level i
	RT_Chain (const RT_Chain &c);
  RT_Chain (ChainId_t id, vector <DcId_t> &S_u);
};

class Non_RT_Chain: public Chain
{
  public:
	  static const vector<Cpu_t> mu_u; // mu_u[i] will hold the # of cpu units required for placing an RT chain on a DC in level i
	  static const Lvl_t  mu_u_len;
		static const vector<Cost_t> cpuCostAtLvl; // cpuCostAtLvl[i] will hold the cost of placing a non- RT chain on a DC in level i
		static const vector<Cost_t> costAtLvl; // cpuCostAtLvl[i] will hold the cost of placing an RT chain on a DC in level i
    Non_RT_Chain (ChainId_t id, vector <DcId_t> &S_u);
	  Non_RT_Chain (const Non_RT_Chain &c);
};

// Instruct the compiler to identify (and, in particular, hash) Chains based on theirs id only.
class ChainHash {
	public:
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
void insertSorted (list <Chain> &sortedList, const Chain &c); // Insert a chain c to the correct place in the vector, based on its latency tightness.

/*************************************************************************************************************************************************
// Given chainId, assigns to chain the respective chain from the set. returns true iff the requested chain Id was found in the set.
**************************************************************************************************************************************************/
bool findChainInSet   (const UnorderedSetOfChains setOfChains , ChainId_t chainId, Chain &c); 
bool findChainInSet 	(const set<Chain> setOfChains, ChainId_t id, Chain& foundChain); 
/*bool findChainInSet (unordered_set <Chain, ChainHash> setOfChains, ChainId_t chainId, Chain &c)*/
bool eraseChainFromSet (UnorderedSetOfChains &setOfChains, ChainId_t chainId); // Given chainId, erases the respective chain from the set. 

vector<Chain> findChainsByPoa (UnorderedSetOfChains setOfChains, DcId_t poa);

#endif



