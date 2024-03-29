// Configuration parameters, writing to files, and accessory functions.
#include "MyConfig.h"

using namespace omnetpp;
using namespace std;

class MyConfig;

string 	 MyConfig::logFileName, MyConfig::resFileName;
string 	 MyConfig::traceFileName; 
int			 MyConfig::netType;
ofstream MyConfig::logFile, MyConfig::resFile;
char 		 MyConfig::buf[MyConfig::bufSize];
//min feasible cpu for SyncPartResh: {842, 94, 1,	30,}. for Async {1067, 119, 1, 30}
const vector <Cpu_t>  				 MyConfig::nonAugmentedCpuAtLeaf = {1067, 111, 1,	30,};  
const vector <vector <Cost_t>> MyConfig::RtChainCostAtLvl      = {{544, 278, 164}, 							{544, 278, 164},						 {100, 10   }, {68, 40, 31}};
const vector <vector <Cost_t>> MyConfig::NonRtChainCostAtLvl   = {{544, 278, 148, 86, 58, 47}, 	{544, 278, 148, 86, 58, 47}, {100, 10, 1}, {68, 40, 29}};
const vector <vector <Cpu_t>>  MyConfig::RtChainMu_u 				   = {{17, 17, 19}, 								{17, 17, 19},								 {1, 	1 	 },  {17, 17, 19}};
const vector <vector <Cpu_t>>  MyConfig::NonRtChainMu_u 		   = {{17, 17, 17, 17, 17, 17},			{17, 17, 17, 17, 17, 17}, 	 {1, 	1, 	1},  {17, 17, 17}};


/*************************************************************************************************************************************************
Inline functions
**************************************************************************************************************************************************/
inline bool MyConfig::fileExists (const std::string& name) // returns true iff the given fileName already exists
{
	struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

/*************************************************************************************************************************************************
Given a cntrNum, increment the respective pktCnt 1, and the respective bitCnt by the given bitCnt.
Return true if the counters were successfully update, false else.
**************************************************************************************************************************************************/
bool MyConfig::incCntr (Lvl_t cntrNum, int64_t bitCnt)
{
	if (MyConfig::isFirstPeriod) { // ignore data xmt in the first period, which is used only to generate the initial sol
		return true;
	}
	
	if (MyConfig::DEBUG_LVL>0 && (cntrNum<0 || cntrNum>2*MyConfig::height-3)) {
		sprintf (buf, "MyConfig::incCntr was called with cntrNum=%d, while maximal allowed cntrNum is %d", cntrNum, 2*MyConfig::height-3);
		MyConfig::printToLog (buf);
		return false;
	}
	MyConfig::pktCnt[cntrNum]++;
	MyConfig::bitCnt[cntrNum]+=bitCnt;
	return true;
}


/*************************************************************************************************************************************************
Given a cntrNum, increment the respective pktCnt 1, and the respective bitCnt by the given bitCnt.
Return true if the counters were successfully update, false else.
**************************************************************************************************************************************************/
int MyConfig::getOverallNumPkts ()
{
	int overallNumPkts = 0;
	for (auto cntr : MyConfig::pktCnt) {
		overallNumPkts += cntr;
	}
	return overallNumPkts;
}


/*************************************************************************************************************************************************
* Run a given (Linux) command, and return its output
**************************************************************************************************************************************************/
string MyConfig::exec(const char* cmd)
{
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

/*************************************************************************************************************************************************
* Init parameters and variables is currently done mainly in SimController.cc
**************************************************************************************************************************************************/
void MyConfig::init()
{
}

/*************************************************************************************************************************************************
* rst parameters before starting a trace run
**************************************************************************************************************************************************/
void MyConfig::rst()
{
	MyConfig::traceTime 				  	= -1.0;
	MyConfig::overallNumBlockedUsrs = 0; 
	MyConfig::discardAllMsgs 				= false;
	MyConfig::lvlOfHighestReshDc 		= UNPLACED_LVL;
	for (int i(0); i<MyConfig::pktCnt.size(); i++) {
		MyConfig::pktCnt[i] = 0;
		MyConfig::bitCnt[i] = 0;
	}
}

/*************************************************************************************************************************************************
* Close the log and result files.
* To be called by the end of sim. If this is a part of a sim campain, the file will be opened at the next iter.
* However, closing thf file is good for sync, consistency, and skipping if the exact same settings was already successfully tried.
**************************************************************************************************************************************************/
void MyConfig::closeFiles ()
{
	resFile					.close ();
	RtProbSimResFile.close (); 
	comOhResFile    .close ();
}


/*************************************************************************************************************************************************
* Open the log and result files.
* Check whether the actual traceFileName corresponds with the netType (set according to the .ini file). 
* For instance, if the .ini file is "Lux.ini", then traceFileName must begin with "Lux".
* Returns true iff the files where successfully opened, and the traceFileName staisfies a basic sanity check.
**************************************************************************************************************************************************/
bool MyConfig::openFiles()
{
	logFile.open (logFileName);
	
	if (fileExists (resFileName)) {
	  resFile.open(resFileName, std::ios_base::app);	
	}
	else {
		resFile.open(resFileName);		  
		resFile << "// format: t{T}.{Mode}.cpu{C}.stts{s} | cpu_cost=... | link_cost=... | mig_cost=... | cost=... | ratio=[c,l,m] c | resh=lvl, , where" << endl;
		resFile << "// T is the slot cnt (read from the input file)" << endl;
		resFile << "// Mode is the algorithm / solver used. Possble modes are:"  << endl;
		resFile << "// AsyncBlk allows blocking new requests, while AsyncNBlk doesn't. SyncPartResh, SyncFullResh allow partial, full reshuffling, resp."  << endl;
		resFile << "// C is the num of CPU units used in the leaf"  << endl;
		resFile << "// [c,l,m] are the ratio of the cpu, link, and mig cost out of the total cost, resp."  << endl;
		resFile << "// lvl is the level of the highest reshuffling datacenter if the alg' has reshuffled for finding a solution at this slot, -1 else."  << endl  << endl;
	resFile << endl << "// BU_ACCUM_DELAY_OF_LVL0 = " << MyConfig::PUSH_DWN_DELAY_OF_LVL[0] << " , PUSH_DWN_DELAY_OF_LVL0 = " << MyConfig::PUSH_DWN_DELAY_OF_LVL[0] << endl; 
	}

	if (logComOh) {
		if (fileExists (comOhResFileName)) {
		  comOhResFile.open(comOhResFileName, std::ios_base::app);
		}
		else {
			comOhResFile.open(comOhResFileName);
			comOhResFile << "// format: t{T}.{Mode}.cpu{C}.stts{s}_ad{ad}_pdd{pdd} | nPkts0=... | nPkts1=... | nBytes0=... |  , where" << endl;
			comOhResFile << "// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where" << endl;
			comOhResFile << "// T is the slot cnt (read from the input file)" << endl;
			comOhResFile << "// Mode is the algorithm / solver used."  << endl;
			comOhResFile << "// ad, pdd are the accumulation delay, push-down delay, resp., in micro-sec."  << endl;
			comOhResFile << "// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.\n";
			comOhResFile << "// The directions are determined as follows:\n";
			comOhResFile << "// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).\n";
			comOhResFile << "// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.\n\n";
		}
	}

	if (MyConfig::runningRtProbSim) {
		sprintf (RtProbSimResFileName, "RtProb_%s_%s_1secs.res", cityName, modeStr);
		if (fileExists (RtProbSimResFileName)) {
			RtProbSimResFile.open(RtProbSimResFileName, std::ios_base::app);	
		}
		else {
			RtProbSimResFile.open(RtProbSimResFileName, std::ios_base::app | std::ios_base::in);
			RtProbSimResFile << 
				"// format: t{T}.{Mode}.cpu{C}.stts{s} | cpu_cost=... | link_cost=... | mig_cost=... | cost=... | ratio=[c,l,m] c | resh=lvl, , where" << endl;
			RtProbSimResFile << "// T is the slot cnt (read from the input file)" << endl;
			RtProbSimResFile << "// Mode is the algorithm / solver used. Possble modes are:"  << endl;
			RtProbSimResFile << 
				"// AsyncBlk allows blocking new requests, while AsyncNBlk doesn't. SyncPartResh, SyncFullResh allow partial, full reshuffling, resp."  << endl;
			RtProbSimResFile << "// C is the num of CPU units used in the leaf"  << endl;
			RtProbSimResFile << "// [c,l,m] are the ratio of the cpu, link, and mig cost out of the total cost, resp."  << endl;
			RtProbSimResFile << 
				"// lvl is the level of the highest reshuffling datacenter if the alg' has reshuffled for finding a solution at this slot, -1 else." << endl;
			RtProbSimResFile << 
				"// BU_ACCUM_DELAY_OF_LVL0 = " << MyConfig::PUSH_DWN_DELAY_OF_LVL[0] << " , PUSH_DWN_DELAY_OF_LVL0 = " 
				<< MyConfig::PUSH_DWN_DELAY_OF_LVL[0] << endl << endl; 
		}
	}
	return true;
}


/*************************************************************************************************************************************************
* parses the given string, and set MyConfig::netType accordingly.
* The currently used netTypes are: "Lux", "Monaco", "UniformTree" and "NonUniformTree". 
* The string is assumed to begin with the NetType string. 
* E.g., string "Lux_xyz_rrgrg___" will cause this function to return the netType Lux.
* If the extracted value is unknown, the func' returns -1.
* The netTypes are defined in MyConfig.h.
**************************************************************************************************************************************************/
void MyConfig::setNetTypeFromString (string str)
{
	if ( (str.substr(0,3)).compare("Lux")==0) {
		MyConfig::netType = LuxIdx;
	}	
	if ( (str.substr(0,6)).compare("Monaco")==0) {
		MyConfig::netType = MonacoIdx;
	}	
	if ( (str.substr(0,11)).compare("UniformTree")==0) {
		MyConfig::netType = UniformTreeIdx;
	}	
	if ( (str.substr(0,14)).compare("NonUniformTree")==0) {
		MyConfig::netType = NonUniformTreeIdx;
	}	
} 

///*************************************************************************************************************************************************
//**************************************************************************************************************************************************/
void MyConfig::printToLog (vector <vector <int32_t>> mat) {
	for (auto vec: mat) {
		MyConfig::printToLog ("\n");
		MyConfig::printToLog (vec);		
	}
}


/*************************************************************************************************************************************************
**************************************************************************************************************************************************/
//void MyConfig::printToLog (const int *ar) {

//	for (int idx(0); idx<sizeof ar;  idx++) {
//		MyConfig::printToLog (ar[idx]);
//	}
//}

/*************************************************************************************************************************************************
* parses the given string, and extract the netType.
* The currently used netTypes are: "Lux", "Monaco", "UniformTree" and "NonUniformTree". 
* The string is assumed to begin with the NetType string. 
* E.g., string "Lux_xyz_rrgrg___" will cause this function to return the netType Lux.
* If the extracted value is unknown, the func' returns -1.
* The netTypes are defined in MyConfig.h.
**************************************************************************************************************************************************/
int MyConfig::getNetTypeFromString (string str)
{
	if ( (str.substr(0,3)).compare("Lux")==0) {
		return LuxIdx;
	}	
	if ( (str.substr(0,6)).compare("Monaco")==0) {
		return MonacoIdx;
	}	
	if ( (str.substr(0,11)).compare("UniformTree")==0) {
		return UniformTreeIdx;
	}	
	if ( (str.substr(0,14)).compare("NonUniformTree")==0) {
		return NonUniformTreeIdx;
	}	
	return -1;
}
void MyConfig::printSuToLog (Chain chain)
{
	printToLog (chain.S_u);
}

// print the IDs of all the chains in the set 
void MyConfig::printToLog (list <Chain> list2print, bool printLvl)
{
	if (printLvl) {
		for (auto chain : list2print) {
			logFile << "c" << chain.id << " l=" << (int)chain.curLvl << ", ";
		}	
	}
	else {
		for (auto chain : list2print) {
			logFile << chain.id << ",";
		}
	}
}

// print all the chain's data
void MyConfig::printToLog (Chain chain)
{
	logFile << "c" << chain.id << ", l=" << chain.curLvl << ", curDc=" << chain.curDc << " S_u="; 
	printToLog (chain.S_u);
}

// print the IDs of all the chains in the set 
void MyConfig::printToLog (UnorderedSetOfChains set2print)
{
	logFile << ",";
	for (auto chain : set2print) {
		logFile << chain.id << ",";
	}
}

//void MyConfig::printToLog (SetOfChainsOrderedByDecCpuUsage setOfChains, bool printS_u)
//{
//	for (auto chain : setOfChains) {
//		logFile << chain.id << ",";
//	}
//}


void MyConfig::printToRes (char* buf) 
{
	resFile << buf; 
}

void MyConfig::printToRes (string str) 
{
	resFile << str;
}

void MyConfig::printToLog (char* buf, ofstream outFile) 
{
	outFile << buf; 
}

void MyConfig::printToLog (char* buf) 
{
	logFile << buf; 
}

void MyConfig::printToLog (string str) 
{
	logFile << str;
}

void MyConfig::printToLog (const int d) 
{
	logFile << d << ",";
}

void MyConfig::printToLog (vector <Chain> vec) 
{
	logFile  << ",";
	for (const auto &chain : vec) {
		logFile << "c" << chain.id << ",";
	}
}

void MyConfig::printToLog (vector <ChainId_t> vec) 
{
	logFile << ",";
	for (const auto chainId : vec) {
		logFile << "c" << chainId << ",";
	}
}

void MyConfig::printToLog (unordered_set <ChainId_t> set2print) 
{
	vector <ChainId_t> setAsVec;
	for (auto const &chainId : set2print) {
		setAsVec.push_back (chainId);
	}
	sort(setAsVec.begin(), setAsVec.end()); 
	MyConfig::printToLog (setAsVec);	
}

vector<Cost_t> MyConfig::scalarProdcut (const vector<Cpu_t> &vec1, const vector<Cost_t> &vec2) 
{

	Lvl_t size = min (vec1.size(), vec2.size()); 
	vector <Cost_t> res; 
	for (int i(0); i<size; i++) {
		res.insert(res.begin()+i, vec1[i] * vec2[i]);
	}
	return res;
}

// erase the given key from the given set. Returns true iff the requested key was indeed found in the set
bool MyConfig::eraseKeyFromSet (unordered_set <ChainId_t> &set, ChainId_t id) 
{
	auto search = set.find (id);

	if (search==set.end()) {
		return false;
	}
	else {
		set.erase(search);
		return true;
	}
}



