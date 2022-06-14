// Configuration parameters, writing to files, and accessory functions.

#include "MyConfig.h"

using namespace omnetpp;
using namespace std;

class MyConfig;

string 					MyConfig::LogFileName, MyConfig::ResFileName;
//string 					MyConfig::traceFileName = "results/poa_files/Monaco_0829_0830_20secs_Telecom.poa";
ofstream 				MyConfig::logFile, MyConfig::ResFile;
char 						MyConfig::buf[MyConfig::bufSize];

/*************************************************************************************************************************************************
* Open the log and result files.
* Check whether the actual traceFileName corresponds with the netType (set according to the .ini file). 
* For instance, if the .ini file is "Lux.ini", then traceFileName must begin with "Lux".
* Returns true iff the files where successfully opened, and the traceFileName staisfies a basic sanity check.
**************************************************************************************************************************************************/
bool MyConfig::openFiles()
{
	LogFileName = "example.txt";
	ResFileName = "res.res";
	logFile.open (LogFileName);
	ResFile.open (ResFileName);
	int traceNetType = getNetTypeFromString (traceFileName);
	if (traceNetType!=MyConfig::netType) {
		return false;
	}
	return true;
}

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
	return -1; // 
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
			logFile << "id=" << chain.id << " l=" << (int)chain.curLvl << ", ";
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
	logFile << "chain " << chain.id << " curDc=" << chain.curDc << " lvl=" << chain.curLvl << " S_u="; 
	printToLog (chain.S_u);
}

// print the IDs of all the chains in the set 
void MyConfig::printToLog (UnorderedSetOfChains set2print)
{
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
	ResFile << buf; 
}

void MyConfig::printToRes (string str) 
{
	ResFile << str;
}

void MyConfig::printToLog (char* buf) 
{
	logFile << buf; 
}

void MyConfig::printToLog (string str) 
{
	logFile << str;
}

void MyConfig::printToLog (int d) 
{
	logFile << d << ",";
}

void MyConfig::printToLog (vector <Chain> vec) 
{
	for (const auto &chain : vec) {
		logFile << chain.id << ",";
	}
}

void MyConfig::printToLog (vector <ChainId_t> vec) 
{
	for (const auto i : vec) {
		logFile << i << ",";
	}
}

void MyConfig::printToLog (unordered_set <ChainId_t> set2print) 
{
	for (const auto i : set2print) {
		logFile << i << ",";
	}
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

//ofstream MyConfig::getLogFile ()
//{
//	return logFile;
//}
