// Configuration parameters, writing to files, and accessory functions.

#include "MyConfig.h"

using namespace omnetpp;
using namespace std;

vector<Cost_t> MyConfig::RT_ChainCostAtLvl;
vector<Cost_t> MyConfig::Non_RT_ChainCostAtLvl;
string 														MyConfig::LogFileName;
string 														MyConfig::traceFileName = "results/poa_files/Tree_shorter.poa";
ofstream 													MyConfig::logFile;
char 															MyConfig::buf[MyConfig::bufSize];

 // set the parameters of chains - e.g., required CPU, and cost.
void MyConfig::setChainsParams () 
{
	MyConfig::RT_ChainCostAtLvl = {1,2,3,4};
	MyConfig::Non_RT_ChainCostAtLvl = {1,2,3,4};
}

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
