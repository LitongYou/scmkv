#include "htable.h"
#include "dzone.h"
#include "memlayout.h"
#include "key.h"
#include "bitops.h"


uint64_t HashTable::Set(const Key &key) 
{
    uint64_t no = BucketIndex(key.Hash64());
    //loger("key<%s> hash index %ld\n", key.data.c_str(), no);

    if (no >= _size / kBucketSize) 
        return -1;

    uint64_t *entry = (uint64_t *)_virt + no;

    return MemoryStorage::Mstore()->
        dz->AddEntryInHTable(entry, key);
}

uint64_t HashTable::Unset(const Key &key) 
{
    uint64_t no = BucketIndex(key.Hash64());
    //loger("key<%s> hash index %ld\n", key.data.c_str(), no);

    if (no >= _size / kBucketSize) 
        return -1;

    uint64_t *entry = (uint64_t *)_virt + no;

    return MemoryStorage::Mstore()->
        dz->DelEntryInHTable(entry, key);
}

uint64_t HashTable::Get(Key &key) 
{

    uint64_t no = BucketIndex(key.Hash64());

    if (no >= _size / kBucketSize) 
        return -1;

    uint64_t key_loc = *((uint64_t *)_virt + no);
#ifndef NDEBUG
    //clear_bit(63, &key_loc);
    //printf("Set chains %p\n" ,phy2virt(lowbits(48, key_loc)));
#endif
    key.SetChains(key_loc);
    return key_loc;
}

uint64_t HashTable::BucketIndex(uint64_t hash)
{
    return hash & kBucketMask; 
}

