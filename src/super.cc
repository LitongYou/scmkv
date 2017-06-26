#include "super.h"
#include <cstring>
#include "memlayout.h"

char SuperBlock::kMagic[8] = "scmkv";

bool SuperBlock::Load()
{
    uint64_t virt = _virt;

    memcpy(magic, (void *)virt, 8);
    virt += 8;
    if (!memcmp(magic,  kMagic, 8))
        return false;

    nr_merger = *(int *)(virt);
    virt += sizeof(int);

    pagesize =  *(int *)(virt); 
    virt += sizeof(int);

    MemoryStorage  *mstore = MemoryStorage::Mstore(); 

    Zone z = *(Zone *)virt;
    virt += sizeof(Zone);
    mstore->pt = new PageTable(z);

    z = *(Zone *)virt;
    virt += sizeof(Zone);
    mstore->ht = new HashTable(z);

    z = *(Zone *)virt;
    virt += sizeof(Zone);
    mstore->dz = new DataZone(z);

    return true;
}


