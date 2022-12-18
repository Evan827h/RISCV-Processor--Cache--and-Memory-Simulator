#include "sim.h"
#include "membus.h"
#include "cpu.h"
#include <string>
#include<iostream>
using namespace std;

sim::sim(string file0, string file1) {
	membus membus(file0, file1);
}