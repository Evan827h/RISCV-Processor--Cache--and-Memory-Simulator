#ifndef ram_H
#define ram_H
#include<string>
#include<map>
#include<vector>
using namespace std;

class ram
{
private:
    vector<string> mem;
    map<string, int> branchList;
public:
    int load(string file, int startAddr);
    string read(int addr);
    bool write(int addr, string data);
    map<string, int> getBranchList();
    vector<string> getMemory();
};


#endif