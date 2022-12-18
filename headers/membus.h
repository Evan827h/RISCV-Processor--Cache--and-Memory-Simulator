#ifndef membus_H
#define membus_H

#include "cpu.h"
#include <map>
#include <string>
#include <functional>
using namespace std;

class cpu;

class membus
{
private:
	int tick = 0;
	ram ram0;
	bool cpu0Finished = false;
	bool cpu1Finished = false;
	bool exBusy = false;

	int clock;

public:
	membus(string file0, string file1);
	pair<int, map<string, int>> load(string file, int startAddr);
	string readMemory(int addr);
	bool writeMemory(int addr, string data);
	void run(cpu& cpu0, cpu& cpu1);
	void validateAdd();
	void validateSub();
	int getClock() { return clock; };
};

#endif
