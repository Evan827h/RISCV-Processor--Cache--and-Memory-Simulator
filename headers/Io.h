#ifndef IO_H
#define IO_H

#include<string>
#include <vector>
using namespace std;

class IO
{
private:

public:
	void print(string stageName, string instrName, string clkCycleNum);
	void print(string stageName, vector<string> IR, string clkCycleNum);

};


#endif