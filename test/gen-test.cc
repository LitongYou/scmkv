#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <random>

#include "gen.h"

using namespace std;

void worker() {
    const size_t nops = 1ul << 25;
    Generator<uint64_t> *gi = new UniformGenerator<uint64_t>(999999999ul);
    //Generator<uint64_t> *gi = new ZipfianGenerator(10000ul, 9999999ul);

    auto start = chrono::system_clock::now();

    for (size_t i = 0; i < nops; ++ i) {
        uint64_t key = gi->next();
        //printf("%010lu\n", key);
    }
        
    chrono::duration<double> dur = chrono::system_clock::now() - start;
    printf("Mops %.2f\n", nops / dur.count() / 1000000);
    delete gi;
}

int main (int argc, char *argv[]){
    int nthread = stoi(string(argv[1]));
    for (int i = 0; i < nthread; ++ i)
        thread(worker).detach();
	this_thread::sleep_for(10s);
    return 0;
}

