#ifndef MY_TYPES_H
#define MY_TYPES_H
using namespace std;


typedef int32_t ChainId_t;
typedef int32_t DcId_t;
typedef int32_t  Lvl_t;
typedef int32_t Cpu_t;
typedef int32_t Cost_t;

const ChainId_t DUMMY_CHAIN_ID = -1;
const DcId_t  	UNPLACED_DC  	 = -1;
const Lvl_t   	UNPLACED_LVL	 = -1;
const Cost_t	  UNPLACED_COST  = -1;
const Cpu_t		  UNPLACED_CPU   = -1;

const bool SYNC  = true;
const bool ASYNC = false;

const vector <Cost_t> vec1[2] = {{4, 3, 2, 1}, {7,7,7,7}};

#endif

