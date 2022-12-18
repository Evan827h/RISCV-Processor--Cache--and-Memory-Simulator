#ifndef ALU_H
#define ALU_H

#include <map>
#include <string>
#include <functional>
using namespace std;

class alu
{
private:
    map<string, function<int(int op1, int op2)>> ops;
    map<string, function<float(float op1, float op2)>> fOps;
public:
    alu();
    int run(string op, int op1, int op2);
    float runF(string op, float op1, float op2);
};

#endif