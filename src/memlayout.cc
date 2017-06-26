#include <cstring>
#include "memlayout.h"
#include "log.h"
#include "bitops.h"

MemoryStorage *MemoryStorage::_mstore = nullptr;
uint64_t MemoryStorage::_virt_base = 0;
const char MemoryStorage::kMagicString[8] = "scmkv!";


bool MemoryStorage::Recovery()
{
    if (_mstore)
        return true;

    _virt_base = (_dev.BaseAddr() + kPageSize - 1) / kPageSize * kPageSize;
    _size = _dev.Size() - (_virt_base - _dev.BaseAddr());

    loger("MemoryStorage Recovery: \n\tdevice base %lX,\n\tstorage base %lX\n",
            _dev.BaseAddr(), _virt_base);

    uint64_t virt = _virt_base;

    if (memcmp((void *)virt,  kMagicString, kMagicSize) != 0)
        return false;
    _mstore = this;
    virt += sizeof(kMagicSize);

    _nr_merger = *(uint64_t *)(virt);
    virt += sizeof(uint64_t);

    Zone z = *(Zone *)virt;
    virt += sizeof(Zone);
    pt = new PageTable(z);

    z = *(Zone *)virt;
    virt += sizeof(Zone);
    ht = new HashTable(z);

    z = *(Zone *)virt;
    virt += sizeof(Zone);
    dz = new DataZone(z, &allocater);

    loger("\tdz %lX\n", dz);

    Allocater *pm = (Allocater *)virt;
    allocater = new Allocater(*pm);
    allocater->Redirect(pt, pm); 
    virt += sizeof(Allocater);

    return true;
}

bool MemoryStorage::Create()
{
    _virt_base = (_dev.BaseAddr() + kPageSize - 1) / kPageSize * kPageSize;
    _size = _dev.Size() - (_virt_base - _dev.BaseAddr());
    
    uint64_t virt = _virt_base;

    loger("MemoryStorage Create Start: \n\tdevice base %lX,\n\tstorage base %lX\n",
            _dev.BaseAddr(), _virt_base);

    //1. copy kMagicString
    memcpy((void *)virt, kMagicString, kMagicSize);
    virt += kMagicSize;

    //2. initiate the number of merger threads
    //   init these threads
    *(uint64_t *)(virt) = kMaxConcurrent;
    virt += sizeof(uint64_t);

    //3.a initiate page table
    uint64_t ptable_start = _virt_base + kPageSize;
    uint64_t ptable_items = _size / kPageSize;
    uint64_t ptable_size = ptable_items * sizeof(Page);
    *(Zone *)virt = Zone(ptable_start, ptable_size);
    virt += sizeof(Zone);

    Page *page = (Page *)ptable_start;
    if (ptable_items >=  (1 << 24) ) {
        printf("memory too big\n");
        exit(0);
    }

    for (uint64_t i = 0; i < ptable_items; ++ i) {
        page->next = i + 1;
        ++ page;
    }
    loger("page table start %lX size %lX, pages %d\n", ptable_start, ptable_size, (ptable_size + kPageSize - 1) / kPageSize);

    // page aligned
    ptable_size = (ptable_size + kPageSize - 1) / kPageSize * kPageSize;
    
    //4. initiate hash table
    uint64_t htable_start = ptable_start + ptable_size + kPageSize;
    //uint64_t htable_items = _size / 512 / 16;
    uint64_t htable_items = _size / 512;
    uint64_t htable_size = (htable_items * kBucketSize);

    int leading_zero = __builtin_clzll(htable_size);
    htable_size = 1ul << (64 - leading_zero);

    memset((void *)htable_start, 0, htable_size);
    loger("htable start %lX size %lX pages %d\n", htable_start, htable_size / kBucketSize, htable_size / kPageSize);

    *(Zone *)virt = Zone(htable_start, htable_size);
    virt += sizeof(Zone);

    //5. initiate data zone 
    uint64_t dzone_start = htable_start + htable_size + kPageSize;
    uint64_t dzone_size = _size - (dzone_start - _virt_base);

    *(Zone *)virt = Zone(dzone_start, dzone_size);
    virt += sizeof(Zone);
    loger("data zone start %lX size %lX\n", dzone_start, dzone_size);

    //6. initiate allocater
    int data_pageno = (dzone_start - _virt_base) / kPageSize + 1;
    int last_pageno = ptable_items - 1;
    allocater = new ((void *)virt) Allocater(data_pageno, last_pageno); // in-place constructor
    //allocater->LocatePageTable(pt);
    virt += sizeof(Allocater);

    loger("MemoryStorage Create End: \n\tdevice base %lX,\n\tstorage base %lX\n",
            _dev.BaseAddr(), _virt_base);
    return true;
}

