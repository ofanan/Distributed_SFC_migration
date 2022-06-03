// Configuration parameters, writing to files, and accessory functions.

#include "MyConfig.h"

using namespace omnetpp;
using namespace std;

string 														MyConfig::LogFileName;
ofstream 													MyConfig::logFile;
unordered_set <Chain, ChainHash> 	ChainsMaster::allChains;
char 															MyConfig::buf[MyConfig::bufSize];

void MyConfig::openFiles()
{
	LogFileName = "example.txt";
	logFile.open (LogFileName);
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

void MyConfig::printToLog (vector <DcId_t> vec) 
{
	for (const auto i : vec) {
		logFile << i << ",";
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

	uint8_t size = min (vec1.size(), vec2.size()); 
	vector <Cost_t> res; 
	for (uint8_t i(0); i<size; i++) {
		res.insert(res.begin()+i, vec1[i] * vec2[i]);
	}
	return res;
}

// erase the given key from the given set. Returns true iff the requested key was indeed found in the set
bool MyConfig::eraseKeyFromSet (unordered_set <ChainId_t> &set, uint16_t id) 
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

// Print the PoA of each currently-active user
void MyConfig::printAllChains () //(bool printSu=true, bool printleaf=false, bool printCurDatacenter=false)
{
	printToLog ("\nallChains\n*******************\n");
	printToLog ("format: (c,d), where c is the chain id, and d is the id of its current DC\n");
	
	for (auto chain : ChainsMaster::allChains) {
		snprintf (buf, bufSize, "(%d,%d)", chain.id, chain.getCurDatacenter());
		printToLog (buf);
	}
}

// Print the PoA of each currently-active user
void MyConfig::printAllChainsPoas () //(bool printSu=true, bool printleaf=false, bool printCurDatacenter=false)
{
	printToLog ("\nallChains\n*******************\n");
	printToLog ("format: (c,p), where c is the chain id, and p is the dcId of its poa\n");
	
	for (auto chain : ChainsMaster::allChains) {
		snprintf (buf, bufSize, "(%d,%d)", chain.id, chain.S_u[0]);
		printToLog (buf);
	}
}

//ofstream MyConfig::getLogFile ()
//{
//	return logFile;
//}
