#ifndef CPU_H
#define CPU_H
#include "alu.h"
#include "ram.h"
#include "Io.h"
#include "membus.h"
#include "cache.h"
#include<string>
#include<utility>
#include<map>

class membus;
class Cache;

typedef struct executeData {
    string type;
    string regAddr;
    int memAddr = -1;
    int intData;
    float fData;
};

class cpu
{
private:
    const int f_delay = 4; //delay - 1
    const int m_delay = 9;

    const double floatCycles = double(f_delay) + 1;

    map<string, int> intReg;
    map<string, float> floatReg;
    map<string, int> branchList;

    int PC = 0;
    int clock;
    int instrLength = -1;
    IO io;
    alu ALU;
    int clockLatencyCount = -1;
    int fetchLatencyCount = -1;
    bool aluBusy = false;
    membus& mem;
    Cache& instrCache;
    Cache& dataCache;

    double instrCount = 0;
    double floatCount = 0;



public:
    cpu(membus& mem_, Cache& instr_, Cache& data_);
    void load(string file, int start);
    string fetch();
    vector<string> decode(string fetchData);
    executeData execute(vector<string> IR);
    void store(executeData exData );
    double getCPI();
    int getPC() { return PC; };
    int getInstrLength() { return instrLength; };
    bool getAluBusy() { return aluBusy; };
    int getClock() { return clock; };
    map<string, int> getIntReg() { return intReg; };
    map<string, float> getFloatReg() { return floatReg; };
    IO getIo() { return io; };
    void incrClock() { clock++; };
    void setAluBusy(bool set);
    
    //old
    void run();

};
#endif
