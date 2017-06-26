#include <cstdio>
#include <cstring>
#include "cpuinfo/cpu.h"


#define BLOCK (2ul << 20)
#define NR_BLOCK (1ul << 10)
#define CLINE (64)

char g_data[CLINE] = "adaf";

void writeblock(char *p, int persist) {
    size_t i = 0;
    for (i = 0; i < BLOCK / CLINE; ++ i) {
        char *dst = p + i * CLINE;
        memcpy(dst, g_data, CLINE);
        if (persist ==  1)
            flush_clflushopt(dst, CLINE);
        if (persist ==  2)
            flush_clflush(dst, CLINE);
    }
}

int main(int argc, char *argv[]) {

    char *p = new char[4ul << 30];
    writeblock(p, false);

    if (argv[1][0] == 'b') {
        for (size_t i = 1; i < NR_BLOCK; ++ i) {
            flush_clflushopt(p + BLOCK * (i - 1), BLOCK);
            writeblock(p + BLOCK * i, 0);
        }
    } else if (argv[1][0] = 'f') {
        for (size_t i = 1; i < NR_BLOCK; ++ i) {
            flush_clflush(p + BLOCK * (i - 1), BLOCK);
            writeblock(p + BLOCK * i, 0);
        }
    } else if (argv[1][0] = 'e') {
        for (size_t i = 1; i < NR_BLOCK; ++ i) {
            flush_clflush(p + BLOCK * (i - 1), BLOCK);
            writeblock(p + BLOCK * i, 2);
        }
    } else {
        for (size_t i = 0; i < NR_BLOCK; ++ i) {
            writeblock(p + BLOCK * i, 1);
        }
    }

    delete []p;
    return 0;
}
