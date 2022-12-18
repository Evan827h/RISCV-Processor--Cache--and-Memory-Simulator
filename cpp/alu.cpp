#include "alu.h"
#include<string>
#include<map>
#include<iostream>
#include <stdint.h>

alu::alu() {
    //logical
    ops.emplace("add", [](int op1, int op2) { return op1 + op2; });
    ops.emplace("addi", [](int op1, int op2) { return op1 + op2; });
    ops.emplace("sub", [](int op1, int op2) { return op1 - op2; });
    ops.emplace("and", [](int op1, int op2) { return op1 & op2; });
    ops.emplace("andi", [](int op1, int op2) { return op1 & op2; });
    ops.emplace("or", [](int op1, int op2) { return op1 | op2; });
    ops.emplace("ori", [](int op1, int op2) { return op1 | op2; });
    ops.emplace("xor", [](int op1, int op2) { return op1 ^ op2; });
    ops.emplace("xori", [](int op1, int op2) { return op1 ^ op2; });
    ops.emplace("lui", [](int op1, int op2) { return op1; }); //adds immidate to 0
    ops.emplace("auipc", [](int op1, int op2) { return op1 + op2; }); //adds imidiate to PC op2 = pc
    ops.emplace("sll", [](int op1, int op2) { return op1 << op2; });
    ops.emplace("slli", [](int op1, int op2) { return op1 << op2; });
    ops.emplace("srl", [](int op1, int op2) { return (unsigned int)op1 >> (unsigned int)op2; }); //logical shift right only works on unsigned variables
    ops.emplace("srli", [](int op1, int op2) { return (unsigned int)op1 >> (unsigned int)op2; });
    ops.emplace("sra", [](int op1, int op2) { return op1 >> op2; });
    ops.emplace("srai", [](int op1, int op2) { return op1 >> op2; }); //set less than signed
    ops.emplace("slt", [](int op1, int op2) { return op1 < op2; });
    ops.emplace("slti", [](int op1, int op2) { return op1 < op2; });
    ops.emplace("sltu", [](int op1, int op2) { return op1 < op2; });
    ops.emplace("sltiu", [](int op1, int op2) { return op1 < op2; });

    //control
    ops.emplace("beq", [](int op1, int op2) { return op1 == op2; });
    ops.emplace("bne", [](int op1, int op2) { return op1 != op2; });
    ops.emplace("blt", [](int op1, int op2) { return op1 < op2; });
    ops.emplace("bge", [](int op1, int op2) { return op1 >= op2; });
    ops.emplace("bltu", [](int op1, int op2) { return op1 < op2; });
    ops.emplace("bgeu", [](int op1, int op2) { return op1 >= op2; });
    ops.emplace("jal", [](int op1, int op2) { return op1 + op2; });
    ops.emplace("jalr", [](int op1, int op2) { return op1 + op2; });

    //float ops
    fOps.emplace("fadd.s", [](float op1, float op2) { return op1 + op2; });
    fOps.emplace("fsub.s", [](float op1, float op2) { return op1 - op2; });
}

int alu::run(string op, int op1, int op2) {
    if(ops.count(op) == 1) {
        return ops[op](op1, op2);
    } else {
        cout << "Error. Operation not found!";
        return -1;
    }
}

float alu::runF(string op, float op1, float op2) {
    if (fOps.count(op) == 1) {
        return fOps[op](op1, op2);
    }
    else {
        cout << "Error. Float operation not found!";
        return -1.0f;
    }
}