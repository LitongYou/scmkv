#ifndef _SUPER_H
#define _SUPER_H

#include "zone.h"
#include "memlayout.h"

class SuperBlock : public Zone {
    public:
        SuperBlock(uint64_t virt): Zone(virt, 0) {}

        bool Load();
        bool Store();

        int NumOfMergers();
        int PageSize();

    private:

        char magic[8];
        int nr_merger;
        int pagesize;
        static char kMagic[8];
};

/*
struct super_block_t {
    char magic[kMagicSize];
    uint64_t nr_woker_threads;
    Zone page_table;
    Zone hash_table;
    Zone data_zone;
    Allocater al;
};
*/


#endif
