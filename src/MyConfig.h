// Configuration parameters, writing to files, and accessory functions.

#ifndef MY_CONFIG_H
#define MY_CONFIG_H
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <regex>
#include <utility>
#include <cstdlib>

#include <vector>
#include <set>
#include <algorithm>
#include <unordered_set>

using namespace omnetpp;
using namespace std;

const int HEIGHT=5;

const int16_t nonAugmentedCpuAtLvl[HEIGHT] = {10,20,30,40,50};

const int16_t chainMigCost = 600;

const int16_t costOfPlacingChainAtLvl[HEIGHT] = {10,20,30,40,50};

const int16_t UNPLACED = -1;

const bool SYNC  = true;
const bool ASYNC = false;

const vector<uint16_t> cpuCostAtLvl = {5,4,3,2,1}; 

class MyConfig { //: public cSimpleModule {

	public:
 		static string LogFileName;
    static ofstream logFile;
		const static bool mode=SYNC;

		//Init
		static void openFiles ();
		static void finish    ();
		
		static ofstream getLogFile ();
/*		void initialize();*/

		//print
		static void printToLog (char* buf); 
		static void printToLog (string str);
		static void printToLog (int d); // print "d,", where d is the integer, to the log file
		static void printToLog (vector <uint16_t> vec); // print a vec of integers to the log file
		static void printToLog (vector <uint32_t> vec); // print a vec of integers to the log file
		static void printToLog (unordered_set <uint32_t> set2print);

		// returns the scalar product of two vectors
		static vector<uint16_t> scalarProdcut (const vector<uint8_t> &vec1, const vector<uint16_t> &vec2);
		
		static vector<uint16_t> scalarProdcut (const vector<uint16_t> &vec1, const vector<uint16_t> &vec2);

};

#endif

