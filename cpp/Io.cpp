#include <string>
#include <iostream>
#include <vector>
#include <iomanip>
#include "Io.h"

using namespace std;

void IO::print(string stageName, string instr, string clkCycleNum) {

    cout << "Stage: " << stageName << "\t" << instr << "\t\t" << setw(15) << "Clock cycle: " << clkCycleNum << "\n";
    
}

void IO::print(string stageName, vector<string> IR, string clkCycleNum) {

    cout << "Stage: " << stageName << "\t";
    for (string s : IR) {
        cout << s << " ";
    }
    cout << setw(15) << "\tClock cycle: " << clkCycleNum << "\n";
}



