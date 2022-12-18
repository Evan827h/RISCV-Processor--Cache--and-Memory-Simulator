#pragma once
#ifndef CACHE_H
#define CACHE_H
#include "membus.h"
#include<string>
#include<utility>
#include<vector>
#include<string>
#include <deque>
#include<tuple>
using namespace std;

class membus;

class Cache
{
private:

	const int addressSize = 32;
	const int m_delay = 9;
	
	int clock;

	int cacheSize;
	int blockSize;
	int assoc;
	int lookupTime;

	int offsetBits;
	int totalSets;
	int setBits = -1;
	int tagBits;
	bool fullyAssoc = false;

	int memLatencyCount = -1;
	double missCount = 0;


	vector<deque<int>> LRU;
	//valid, tag, data
	vector<vector<tuple<bool, int, vector<string>>>> ways;

	int hitCount = 0;
	int lineCount = 0;
	int coldCount = 0;
	int totalLookupTime = 0;

	bool write(int address, string data);
	pair<bool, string> read(int address);
	int getSet(int address);
	int getTag(int address);
	int getOffset(int address);
	void printCache();

	membus& bus;

public:
	Cache(membus& bus_, int cacheSize, int blockSize, int a);
	bool run(int address, string data);
	string run(int address);
	double getMissCount() { return missCount; };
	double getTotalLookupTime() { return double(totalLookupTime); };
	void printStats();
};
#endif
