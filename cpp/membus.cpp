#include "cpu.h"
#include "ram.h"
#include "membus.h"
#include "cache.h"
#include <string>
#include<iostream>
using namespace std;

membus::membus(string file0, string file1) {
    
    
    Cache iCache0(*this, 256, 128, 1);
    Cache dCache0(*this, 512, 128, 1);

    Cache iCache1(*this, 256, 128, 1);
    Cache dCache1(*this, 512, 128, 1);

    cpu cpu0(*this, iCache0, dCache0);
    cpu cpu1(*this, iCache1, dCache1);

    cpu0.load(file0, 0);
    cpu1.load(file1, 0x100);

    clock = cpu0.getClock();

    run(cpu0, cpu1);
    validateAdd();
    validateSub();

    cout << "\n\ncpu0 CPI: " << cpu0.getCPI() << "\n\n";
    cout << "\ncpu1 CPI: " << cpu1.getCPI() << "\n\n";
    iCache0.printStats();
    dCache0.printStats();
    

    cout << "all finished";

}

pair<int, map<string, int>> membus::load(string file, int startAddr) {
     int c = ram0.load(file, startAddr);
     map<string, int> branches = ram0.getBranchList();
     return make_pair(c, branches);
}

string membus::readMemory(int addr) {
    return ram0.read(addr);
}

bool membus::writeMemory(int addr, string data) {
    return ram0.write(addr, data);
}

void membus::run(cpu& cpu0, cpu& cpu1) {
    string fe0;
    string fe1;
    vector<string> de0;
    vector<string> de1;
    executeData ex0;
    executeData ex1;

    while (!cpu0Finished || !cpu1Finished) {
        cout << "Running." << "\n";

        //store
        if (!cpu0Finished) {
            cpu0.store(ex0);
        }
        if (!cpu1Finished) {
            cpu1.store(ex1);
        }

        //execute
        if (!cpu0Finished) {
            ex0 = cpu0.execute(de0);
        }
        if (!cpu1Finished) {
            executeData checkEx1 = cpu1.execute(de1);
            if (ex0.memAddr != -1 && ex0.memAddr == checkEx1.memAddr) {
                exBusy = true;
                cpu0.getIo().print("Execute", "Stalled", to_string(cpu0.getClock()));
            }
            else {
                ex1 = checkEx1;
            }

        }
        
        //decode
        if (!cpu0.getAluBusy() && !cpu0Finished) {
            de0 = cpu0.decode(fe0);
        }
        else  if (!cpu0Finished) {
            cpu0.getIo().print("Decode", "Stalled", to_string(cpu0.getClock()));
        }
        
        if (!cpu1.getAluBusy() && !cpu1Finished && !exBusy) {
            de1 = cpu1.decode(fe1);
        }
        else  if (!cpu1Finished && !exBusy) {
            cpu1.getIo().print("Decode", "Stalled", to_string(cpu1.getClock()));
        }

        //fetch
        if (!cpu0.getAluBusy() && !cpu0Finished) {
            fe0 = cpu0.fetch();
        }
        else if (!cpu0Finished) {
            cpu0.getIo().print("Fetch", "Stalled", to_string(cpu0.getClock()));
        }
        if (!cpu1.getAluBusy() && !cpu1Finished && !exBusy) {
            fe1 = cpu1.fetch();
        }
        else if (!cpu1Finished && !exBusy) {
            cpu1.getIo().print("Fetch", "Stalled", to_string(cpu1.getClock()));
        }


        cpu0.incrClock();
        cpu1.incrClock();
        clock = cpu0.getClock();
        if (cpu0.getPC() >= cpu0.getInstrLength() + 2) {
            cpu0Finished = true;
        }
        if (cpu1.getPC() >= cpu1.getInstrLength() + 2) {
            cpu1Finished = true;
        }
        exBusy = false;
    }
    cout << "\n\nCPU 0 regs:\n";
    
    cout << "\nInt Registers:\n";
    for (int i = 0; i < 32; i++) {
        string s = "x" + to_string(i);
        cout << s << " " << cpu0.getIntReg().at(s) << "\n";
    }
    cout << "\nFloat Registers:\n";
    for (int i = 0; i < 32; i++) {
        string s = "f" + to_string(i);
        cout << s << " " << cpu0.getFloatReg().at(s) << "\n";
    }
    cout << "\n\nCPU 1 regs:\n";
    
    cout << "\nInt Registers:\n";
    for (int i = 0; i < 32; i++) {
        string s = "x" + to_string(i);
        cout << s << " " << cpu1.getIntReg().at(s) << "\n";
    }
    cout << "\nFloat Registers:\n";
    for (int i = 0; i < 32; i++) {
        string s = "f" + to_string(i);
        cout << s << " " << cpu1.getFloatReg().at(s) << "\n";
    }
    
    cout << "Done." << "\n";
}

void membus::validateAdd() {
#define ARRAY_A_ADDR 0x0400
#define ARRAY_B_ADDR 0x0800
#define ARRAY_C_ADDR 0x0C00

    float* ARRAY_A = (float*)(void*)ARRAY_A_ADDR;
    float* ARRAY_B = (float*)(void*)ARRAY_B_ADDR;
    float* ARRAY_C = (float*)(void*)ARRAY_C_ADDR;

    cout << "\n\nValidating Results:\n";

    int i;
    for (i = 0; i < 256; i++) {
        float expected = (stof(ram0.read(ARRAY_A_ADDR + i)) + stof(ram0.read(ARRAY_B_ADDR + i)));
        float actual = stof(ram0.read(ARRAY_C_ADDR + i));
        
        if (expected == actual) {
            cout << "Passed!\tExpected: " << expected << "\tActual: " << actual << "\n";
        }
        else {
            cout << "failed!\tExpected: " << expected << "\tActual: " << actual << "\n";
        }
    }
}

void membus::validateSub() {
#define ARRAY_A_ADDR 0x0400
#define ARRAY_B_ADDR 0x0800
#define ARRAY_C_ADDR 4096

    float* ARRAY_A = (float*)(void*)ARRAY_A_ADDR;
    float* ARRAY_B = (float*)(void*)ARRAY_B_ADDR;
    float* ARRAY_C = (float*)(void*)ARRAY_C_ADDR;

    cout << "\n\nValidating Results:\n";

    int i;
    for (i = 0; i < 256; i++) {
        float expected = (stof(ram0.read(ARRAY_A_ADDR + i)) - stof(ram0.read(ARRAY_B_ADDR + i)));
        float actual = stof(ram0.read(ARRAY_C_ADDR + i));

        if (expected == actual) {
            cout << "Passed!\tExpected: " << expected << "\tActual: " << actual << "\n";
        }
        else {
            cout << "failed!\tExpected: " << expected << "\tActual: " << actual << "\n";
        }
    }
}


