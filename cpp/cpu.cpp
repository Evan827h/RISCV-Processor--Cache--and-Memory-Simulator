#include "cpu.h"
#include "Io.h"
#include<string>
#include <sstream>
#include<iostream>
#include<vector>
#include<utility>


using namespace std;



cpu::cpu(membus& mem_, Cache& instr_, Cache& data_) : mem(mem_), instrCache(instr_), dataCache(data_) {
    
    //build registers
    for (int i = 0; i < 32; i++) {
        string x = "x" + to_string(i);
        string f = "f" + to_string(i);
        intReg.emplace(x, 0);
        floatReg.emplace(f, 0.0f);
    }
    intReg["x1"] = 1;
    floatReg["f1"] = 1;
    clock = 0;
}

void cpu::load(string file, int start) {
    //load instriction file
    cout << "loading Instriction file: " << file << "\n";
    auto res = mem.load(file, start);
    instrLength = res.first;
    branchList = res.second;
    PC = start;
}

string cpu::fetch() {
    string fetchData = instrCache.run(PC);
    if (fetchData != "") {
        PC++;
    } 
    io.print("Fetch", fetchData, to_string(clock));
    return fetchData;
}

vector<string> cpu::decode(string fetchData) {
    vector<string> IR;
    
    if (fetchData == "") {
        io.print("Decode", "Inactive", to_string(clock));
        return IR;
    }
    
    istringstream iss(fetchData);
    string op;
    while (getline(iss, op, ' ')) {
        IR.push_back(op);
    }
    io.print("Decode", IR, to_string(clock));
    return IR;
}

executeData cpu::execute(vector<string> IR) {
    executeData exData;
    if (IR.empty()) {
        io.print("Execute", "Inactive", to_string(clock));
        return exData;
    }

    if (clock < clockLatencyCount && clockLatencyCount != -1) {
        io.print("Execute", IR, to_string(clock));
        return exData;
    }

    
    string opCode = IR[0];
    //basic  logic
    if (opCode == "add" || opCode == "sub" || opCode == "and" || opCode == "or" || opCode == "xor" || opCode == "sll" || opCode == "srl" || opCode == "sra" || opCode == "slt" || opCode == "sltu") {
        exData.type = "regStore";
        exData.regAddr = IR[1];
        exData.intData =  ALU.run(opCode, intReg[IR[2]], intReg[IR[3]]);
        instrCount++;
    }
    //immidiate logic
    if (opCode == "addi" || opCode == "andi" || opCode == "ori" || opCode == "xori" || opCode == "slli" || opCode == "srli" || opCode == "srai" || opCode == "slti" || opCode == "sltiu") {
        exData.type = "regStore";
        exData.regAddr = IR[1];
        exData.intData = ALU.run(opCode, intReg[IR[2]], stoi(IR[3]));
        instrCount++;
    }
    if (opCode == "lui") {
        exData.type = "regStore";
        exData.regAddr = IR[1];
        exData.intData = ALU.run(opCode, intReg[IR[2]], NULL);
        instrCount++;
    }
    if (opCode == "auipc") {
        exData.type = "regStore";
        exData.regAddr = IR[1];
        exData.intData = ALU.run(opCode, intReg[IR[2]], PC);
        instrCount++;
    }
    //control
    if (opCode == "beq" || opCode == "bne" || opCode == "blt" || opCode == "bge" || opCode == "bltu" || opCode == "bgeu") {
        if (ALU.run(opCode, intReg[IR[1]], intReg[IR[2]])) {
            PC = branchList[IR[3]] - 1;
        }
        instrCount++;
    }
    if (opCode == "j") {
        PC = branchList[IR[1]] - 1;
        instrCount++;
    }
    if (opCode == "jal") {
        if (IR[1] != "x0") {
            exData.type = "regStore";
            exData.regAddr = IR[1];
            exData.intData = ALU.run(opCode, intReg[IR[2]], PC);
            PC = exData.intData - 1;
        }
        else {
            PC = ALU.run(opCode, intReg[IR[2]], PC) - 1;
        }
        instrCount++;
    }
    if (opCode == "jalr") {
        if (IR[1] != "x0") {
            exData.type = "regStore";
            exData.regAddr = IR[1];
            exData.intData = ALU.run(opCode, intReg[IR[2]], intReg[IR[3]]);
            PC += exData.intData - 1;
        }
        else {
            PC += ALU.run(opCode, intReg[IR[2]], intReg[IR[3]]) - 1;
        }
        instrCount++;
    }
    //load store
    if (opCode == "lb" || opCode == "lbu" || opCode == "lh" || opCode == "lhu" || opCode == "lw" || opCode == "lwu" || opCode == "ld") {
        
        int offset = stoi(IR[2].substr(0, IR[2].find('(')));
        int reg = intReg[IR[2].substr(IR[2].find('(') + 1, IR[2].find(')') - 2)];
        int addr = offset + reg;
        exData.type = "regStore";
        exData.regAddr = IR[1];
        try {
            exData.intData = stoi(dataCache.run(addr));
        }
        catch (const invalid_argument& ia) {
            aluBusy = true;
            io.print("Execute", IR, to_string(clock));
            return exData;
        }
        aluBusy = false;
        instrCount++;
    }
    if (opCode == "sb" || opCode == "sh" || opCode == "sw" || opCode == "sd") {
        
        int addr = stoi(IR[2].substr(0, IR[2].find('('))) + intReg[IR[2].substr(IR[2].find('(') + 1, IR[2].find(')') - 2)];
        exData.type = "memStore";
        exData.memAddr = addr;
        exData.intData = intReg[IR[1]];
        if (!dataCache.run(exData.memAddr, to_string(exData.intData))) {
            aluBusy = true;
            io.print("Execute", IR, to_string(clock));
            return exData;
        }
        aluBusy = false;
        instrCount++;
    }

    //float operations
    if (opCode == "fadd.s" || opCode == "fsub.s") {
        if (clockLatencyCount == -1) {
            clockLatencyCount = clock + f_delay;
            aluBusy = true;
            io.print("Execute", IR, to_string(clock));
            return exData;
        }
        clockLatencyCount = -1;
        exData.type = "fRegStore";
        exData.regAddr = IR[1];
        exData.fData = ALU.runF(opCode, floatReg[IR[2]], floatReg[IR[3]]);
        aluBusy = false;
        floatCount++;
        instrCount++;
    }
        //load float
    if (opCode == "flw") {
        if (clockLatencyCount == -1) {
            clockLatencyCount = clock + f_delay;
            aluBusy = true;
            io.print("Execute", IR, to_string(clock));
            return exData;
        }
        
        int offset = stoi(IR[2].substr(0, IR[2].find('(')));
        int reg = intReg[IR[2].substr(IR[2].find('(') + 1, IR[2].find(')') - 2)];
        int addr = offset + reg;
        exData.type = "fRegStore";
        exData.regAddr = IR[1];
        float tawd = stof(mem.readMemory(addr));
        try {
            exData.fData = stof(dataCache.run(addr));
        }
        catch (const invalid_argument& ia) {
            aluBusy = true;
            io.print("Execute", IR, to_string(clock));
            return exData;
        }
        clockLatencyCount = -1;
        aluBusy = false;
        floatCount++;
        instrCount++;
    }
        //store float
    if (opCode == "fsw") {
        if (clockLatencyCount == -1) {
            clockLatencyCount = clock + f_delay;
            aluBusy = true;
            io.print("Execute", IR, to_string(clock));
            return exData;
        }
        
        int addr = stoi(IR[2].substr(0, IR[2].find('('))) + intReg[IR[2].substr(IR[2].find('(') + 1, IR[2].find(')') - 2)];
        exData.type = "fmemStore";
        exData.memAddr = addr;
        exData.fData = floatReg[IR[1]];
        if (!dataCache.run(exData.memAddr, to_string(exData.fData))) {
            aluBusy = true;
            io.print("Execute", IR, to_string(clock));
            return exData;
        }
        clockLatencyCount = -1;
        aluBusy = false;
        floatCount++;
        instrCount++;
    }
    io.print("Execute", IR, to_string(clock));
    return exData;
}

void cpu::store(executeData exData) {
    //store in register
    if (exData.type == "regStore") {
        if (exData.regAddr.front() == 'x') {
            intReg[exData.regAddr] = exData.intData;
        }
        else {
            cout << "Could not find Register " << exData.regAddr << "\n";
        }
        io.print("Store", to_string(exData.intData) + " in " + exData.regAddr, to_string(clock));
    }
     else if (exData.type == "fRegStore") {
        if (exData.regAddr.front() == 'f') {
            floatReg[exData.regAddr] = exData.fData;
        }
        else {
            cout << "Could not find Register " << exData.regAddr << "\n";
        }
        io.print("Store", to_string(exData.fData) + " in " + exData.regAddr, to_string(clock));
    }
    else {
        //cout << "Inactive to Store!\n";
        io.print("Store", "Inactive", to_string(clock));
    }
    
}

void cpu::run() {
    string fe;
    vector<string> de;
    vector<string> deBuff;
    executeData ex;

    while (PC < instrLength + 2) {
        cout << "Running." << "\n";

        store(ex);
        //ex = execute(de);
        if (!aluBusy) {
            de = decode(fe);
            fe = fetch();
        }
        else {
            if (fe == "") {
                io.print("Decode", "Stalled", to_string(clock));
                fe = fetch();
            }
            else {
                io.print("Decode", "Stalled", to_string(clock));
                io.print("Fetch", "Stalled", to_string(clock));
            }
        }
        clock++;
    }

    for (int i = 0; i < 32; i++) {
        string s = "x" + to_string(i);
        cout << s<< " " << intReg.at(s) << "\n";
    }
    for (int i = 0; i < 32; i++) {
        string s = "f" + to_string(i);
        cout << s << " " << floatReg.at(s) << "\n";
    }
    //vector<string> mem = RAM.getMemory();
    cout << "Done." << "\n";
}

double cpu::getCPI() {
    //double instrCount = opCount + memOpCount + floatOpCount + floatMemOpCount;
    //return ((opCount * opCycles) + (memOpCount * memOpCycles) + (floatOpCount * floatOpCycles) + (floatMemOpCount * floatMemOpCycles)) / instrCount;

    double memAccessCount = instrCache.getTotalLookupTime() + dataCache.getTotalLookupTime();

    return ((memAccessCount / 10) + (floatCount * floatCycles)) / instrCount;
}

void cpu::setAluBusy(bool set) {
    aluBusy = set;
}








