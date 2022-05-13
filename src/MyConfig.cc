// Configuration parameters, writing to files, and accessory functions.

#include "MyConfig.h"

using namespace omnetpp;
using namespace std;

//Define_Module(MyConfig);

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

void MyConfig::printToLog (vector <uint16_t> vec) 
{
	for (const auto i : vec) {
		logFile << i << ",";
	}
}

void MyConfig::printToLog (vector <uint32_t> vec) 
{
	for (const auto i : vec) {
		logFile << i << ",";
	}
}

void MyConfig::printToLog (unordered_set <uint32_t> set2print) 
{
	for (const auto i : set2print) {
		logFile << i << ",";
	}
}

vector<uint16_t> MyConfig::scalarProdcut (const vector<uint16_t> &vec1, const vector<uint16_t> &vec2) 
{ 
	vector <uint16_t> res;
	for (uint8_t i(0); i<vec1.size(); i++) {
		res[i] = vec1[i] * vec2[i];
	}
	return res;
}

void MyConfig::finish ()
{
  logFile.close ();
}
