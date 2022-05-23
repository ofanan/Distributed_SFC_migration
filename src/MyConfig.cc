// Configuration parameters, writing to files, and accessory functions.

#include "MyConfig.h"

using namespace omnetpp;
using namespace std;

string 		MyConfig::LogFileName;
ofstream 	MyConfig::logFile;

void MyConfig::openFiles()
{
	LogFileName = "example.txt";
	logFile.open (LogFileName);
}

//ofstream MyConfig::getLogFile ()
//{
//	return logFile;
//}

void MyConfig::printToLog (SetOfChainsOrderedByCpuUsage setOfChains, bool printS_u)
{
	for (auto chain : setOfChains) {
		chain.print (printS_u);
	}
}

void MyConfig::printSuToLog (Chain chain)
{
	printToLog (chain.S_u);
}

void MyConfig::printToLog (UnorderedSetOfChains set2print)
{
	for (auto chain : set2print) {
		logFile << chain.id << ",";
	}
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
	logFile << endl;
}

void MyConfig::printToLog (vector <uint16_t> vec) 
{
	for (const auto i : vec) {
		logFile << i << ",";
	}
	logFile << endl;
}

void MyConfig::printToLog (vector <uint32_t> vec) 
{
	for (const auto i : vec) {
		logFile << i << ",";
	}
	logFile << endl;
}

void MyConfig::printToLog (unordered_set <uint32_t> set2print) 
{
	for (const auto i : set2print) {
		logFile << i << ",";
	}
}

vector<uint16_t> MyConfig::scalarProdcut (const vector<uint16_t> &vec1, const vector<uint16_t> &vec2) 
{

	uint8_t size = min (vec1.size(), vec2.size()); 
	vector <uint16_t> res; // = {size};
	for (uint8_t i(0); i<size; i++) {
		res.insert(res.begin()+i, vec1[i] * vec2[i]);
	}
	return res;
}


