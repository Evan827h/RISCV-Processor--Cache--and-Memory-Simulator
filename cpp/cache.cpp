#include "cache.h"
#include <string>
#include<iostream>
#include <fstream>
#include<cmath>
#include <math.h>
#include<bitset>
#include <deque>
#include <sstream>
#include<tuple>
using namespace std;

Cache::Cache(membus& bus_, int cs, int bs, int a): bus(bus_) {
	cacheSize = cs;
	blockSize = bs;
	assoc = a;
	clock = 0;
	lookupTime = (ceil(log2(cacheSize / blockSize)) * assoc);

	if (assoc > 4) {
		fullyAssoc = true;
		lookupTime = (ceil(log2(cacheSize / blockSize)) * blockSize);
	}

	if (!fullyAssoc) {
		offsetBits = int(log2(blockSize));
		totalSets = cacheSize / (blockSize * assoc);
		setBits = int(log2(totalSets));
		tagBits = addressSize - offsetBits - setBits;
	}
	else {
		offsetBits = int(log2(blockSize));
		totalSets = 1;
		tagBits = addressSize - offsetBits;
	}

	vector<string> dataStart;
	dataStart.resize(blockSize, "NULL");
	vector<tuple<bool, int, vector<string>>> wayStart;
	wayStart.resize(totalSets, make_tuple(false, -1, dataStart));

	if (!fullyAssoc) {
		ways.resize(assoc, wayStart);
	}
	else {
		ways.resize(blockSize, wayStart);
	}

	LRU.resize(totalSets);
	for (int i = 0; i < LRU.size(); i++) {
		for (int j = 0; j < assoc; j++) {
			LRU[i].push_back(j);
		}
	}
}

bool Cache::run(int address, string data) {
	
	clock = bus.getClock();
	
	if (clock < memLatencyCount && memLatencyCount != -1) {
		return false;
	}
	if (memLatencyCount == -1) {
		memLatencyCount = clock + m_delay;
		return false;
	}
	memLatencyCount = -1;
	missCount++;
	if (bus.writeMemory(address, data)) {
		write(address, data);
		return true;
	}
	else
	{
		cout << "Cound not find address " << address << " in RAM!\n";
		return false;
	}
	
	//printCache();
}

string Cache::run(int address) {

	
	clock = bus.getClock();
	auto res = read(address);
	if (res.first && res.second != "NULL") {
		lineCount++;
		hitCount++;
		totalLookupTime += lookupTime;
		return res.second;
	}
	else {
		if (clock < memLatencyCount && memLatencyCount != -1) {
			return "";
		}
		if (memLatencyCount == -1) {
			memLatencyCount = clock + m_delay;
			missCount++;
			return "";
		}
		memLatencyCount = -1;
		lineCount++;
		totalLookupTime += (lookupTime + ((m_delay + 1) * 10));
		write(address, bus.readMemory(address));
		return "";
	}
	//printCache();
}

bool Cache::write(int address, string data) {

	int set = getSet(address);
	int tag = getTag(address);
	int off = getOffset(address);

	int wayIndex = LRU[set].front();

	bool complulsory = false;
	if (get<2>(ways[wayIndex][set])[off] == "NULL") complulsory = true;

	get<0>(ways[wayIndex][set]) = true;
	get<1>(ways[wayIndex][set]) = tag;
	get<2>(ways[wayIndex][set])[off] = data;

	if (complulsory) {
		coldCount++;
	}

	LRU[set].pop_front();
	LRU[set].push_back(wayIndex);

	return false;

}

pair<bool, string> Cache::read(int address) {

	int set = getSet(address);
	int tag = getTag(address);
	int off = getOffset(address);

	int wayIndex = LRU[set].front();

	for (int i = 0; i < ways.size(); i++) {
		if (get<0>(ways[i][set]) == true && get<1>(ways[i][set]) == tag) {
			return make_pair(true, get<2>(ways[i][set])[off]);
		}
	}

	LRU[set].pop_front();
	LRU[set].push_back(wayIndex);

	return make_pair(false, "");

}

int Cache::getSet(int address) {
	if (fullyAssoc) return 0;
	string set = bitset<32>(address).to_string().substr(tagBits, setBits);
	//cout << bitset<32>(address).to_string() << "\n";
	//cout << set << "\n\n";
	return stoi(set,0, 2);
}

int Cache::getTag(int address) {
	string tag = bitset<32>(address).to_string().substr(0, tagBits);
	return stoi(tag, 0, 2);
}

int Cache::getOffset(int address) {
	string off = bitset<32>(address).to_string().substr(tagBits + setBits);
	return stoi(off, 0, 2);
}

void Cache::printCache() {
	if (!fullyAssoc) {
		for (int i = 0; i < totalSets; i++) {
			cout << LRU[i].front() << "\t";
			for (int j = ways.size() - 1; j >= 0; j--) {
				cout << get<0>(ways[j][i]) << "\t" << get<1>(ways[j][i]) << "\t";
			}
			cout << "\n";
		}
		cout << "\n\n";
	}
	else {
		for (int i = ways.size() - 1; i >= 0; i--) {
			cout << get<0>(ways[i][0]) << "\t" << get<1>(ways[i][0]) << "\n";
		}
	}
}

void Cache::printStats() {
	cout << "Cache Size: " << cacheSize << "B\t" << "Cache Line Size: " << blockSize << "B\t" << "Associativity: " << assoc << "\n";
	cout << "Hit Rate: " << hitCount << "/" << lineCount << " : " << (double(hitCount) / double(lineCount)) << "\n";
	cout << "Cold Start Rate: " << coldCount << "/" << lineCount << " : " << (double(coldCount) / double(lineCount)) << "\n";
	cout << "Total Lookup Time: " << totalLookupTime << " Ticks\n\n";
}