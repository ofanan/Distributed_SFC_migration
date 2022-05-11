// Configuration parameters, writing to files, and accessory functions.

#include "MyConfig.h"

using namespace omnetpp;
using namespace std;

//Define_Module(MyConfig);


//string MyConfig::LogFileName = "example.txt";
//ofstream MyConfig::logFile.open (LogFileName);

string 		MyConfig::LogFileName;
ofstream 	MyConfig::logFile;

void MyConfig::openFiles()
{
	LogFileName = "example.txt";
	logFile.open (LogFileName);
}

void MyConfig::printToLog (string str)
{
	logFile << str;
}

vector<uint16_t> MyConfig::scalarProdcut (const vector<uint8_t> &vec1, const vector<uint16_t> &vec2) 
{ 
	vector <uint16_t> res;
	for (uint8_t i(0); i<vec1.size(); i++) {
		res[i] = vec1[i] * vec2[i];
	}
	return res;
}

