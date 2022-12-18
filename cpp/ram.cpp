#include "ram.h"
#include <fstream>
#include<iostream>
#include <stdlib.h> 
#include <time.h>

using namespace std;

int ram::load(string file, int startAddr) {
    ifstream assembly;
    string line;
    assembly.open(file);
    int memIndex = startAddr;
    branchList.clear();

    mem.resize(5119, "0");

    if(!assembly.is_open()) {
        cout << "File cannot Open!\n";
    }

    while (getline(assembly, line)) {
        if (line.front() != '#') {
            mem[memIndex] = line;
            memIndex++;
            if (line.back() == ':') {
                branchList.emplace(line.substr(0, line.length() - 1), memIndex);
            }
        }
    }
    srand(time(NULL));
    //Random numbers for arrays
    for (int i = 0x400; i < 0xC00; i++) {
        
        float f = rand() % 100 + 1;
        mem[i] = to_string(f);
    }

    assembly.close();
    cout << "Successfully loaded file." << "\n";
    return memIndex;
}

string ram::read(int addr) {
    if(addr < mem.size()){
        return mem[addr];
    } else {
        cout << "Could not find address " << addr << " in RAM.\n";
        return "";
    }
}

bool ram::write(int addr, string data) {
    if(addr > 0x093 && addr < 0x13FF) {
        mem[addr] = data;
        return true;
    } else {
        return false;
    }
}

map<string, int> ram::getBranchList() {
    return branchList;
}

vector<string> ram::getMemory() {
    return mem;
}
