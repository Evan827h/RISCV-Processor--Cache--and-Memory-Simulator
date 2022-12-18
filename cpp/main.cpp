#include "sim.h"
#include<string>
using namespace std;

int main()
{
    string file0Name = "./vadd.txt";
    string file1Name = "./vsub.txt";
    sim simulator(file0Name, file1Name);
    return 0;
}