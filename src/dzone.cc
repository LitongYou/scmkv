#include <iostream>
#include <string> 
#include <cstring>
#include <cassert>
#include <cstdio>

#include <sched.h>
#include "list.h"

#include "dzone.h"
#include "page.h"
#include "memlayout.h"
#include "log.h"
#include "atomic.h"
#include "bitops.h"
#include "config.h"
#include "hash/farmhash.h"
#include "cpuinfo/cpu.h"

#include "key.h"

using namespace std;

struct kv_layout {
   uint64_t buff_size;
   uint64_t version;
   uint64_t key_size;
   char     key[0]; //  varlen & 8-byte aligned;
   uint64_t value_size;
   char     value[0]; //  varlen & 8-byte aligned;
   uint64_t checksum;
};

inline struct kv_layout *key_header(const char *pkey)
{
    size_t offset = offsetof(struct kv_layout, key);
    struct kv_layout *p = (decltype(p))(pkey - offset);
    assert(offset == 24);
    return p;
}

// @pkey pointer to key
inline void DataZone::FreeKVItem(const char *pkey)
{
    struct kv_layout *p = key_header(pkey);
    free(p, p->buff_size);
}

static inline void persist_key(const Key &key)
{
    struct kv_layout *p = key_header(key.data());
    clflush(p, p->buff_size);
}

uint64_t DataZone::DelEntryInHTable(uint64_t *entry, const Key &key) 
{
    while(!test_and_set_bit(63, entry))
        ;

    uint64_t old_entry = *entry;

    uint64_t old_pos = lowbits(48, old_entry);
    uint64_t *old_virt = (uint64_t *)phy2virt(old_pos);
    uint64_t bucket_size = *old_virt;

    uint64_t *virt = old_virt + 1;

    clear_bit(63, &old_entry);

    const uint64_t nr_items = highbits(8, old_entry); 
    
    // free lock when update entry
    if (nr_items > 2) {
        for (size_t i = 0; i < nr_items; ++ i) {
            uint64_t phy = lowbits(48, *virt);
            const char *pkey = (decltype(pkey))phy2virt(phy);
            if (pkey == key.data() - 8) {
                memmove(virt, virt + 1, 8 * (nr_items - i - 1));
                //*entry = lowbits(48, *entry) | ((nr_items - 1) << kNrShift);
                set_high_bits(8, nr_items - 1, entry);
                break;
            }
            ++ virt;
        }
    } else if (nr_items == 2) {
        bool is_done = false;

        uint64_t phy = lowbits(48, *virt);
        const char *pkey = (decltype(pkey))phy2virt(phy);
        if (pkey == key.data() - 8) {
            *entry = lowbits(48, *(virt + 1)) | (1ul << kNItemsShift);
            is_done = true;
        }

        phy = lowbits(48, *(virt + 1));
        pkey = (decltype(pkey))phy2virt(phy);
        if (!is_done && pkey == key.data() - 8) {
            *entry = lowbits(48, *(virt - 1)) | (1ul << kNItemsShift);
            is_done = true;
        }

        if (is_done)
            free(old_virt, bucket_size); 
    } else if (nr_items == 1) {
        uint64_t phy = lowbits(48, old_entry);
        const char *pkey = (decltype(pkey))phy2virt(phy);
        if (pkey == key.data() - 8)
            *entry = 0;
    }

    FreeKVItem(key.data());
    return nr_items - 1;
}

// @entry : bucket entry 
// each item in a bucket contains a 16-bits tag and 48-bits phsyic address.
// the first item is the size of the bucket.
// 
uint64_t DataZone::AddEntryInHTable(uint64_t *entry, const Key &key) 
{
    while(!test_and_set_bit(63, entry))
        ;

#if 0
    uint64_t nr_items = highbits(8, *entry); 
    uint64_t pchains = lowbits(48, *entry);
    uint64_t *p = (decltype(p)) phy2virt(pchains);
    uint64_t chains_size;
    int tid = sched_getcpu();

    if (!nr_items) {
        nr_items = 1;
        p = (uint64_t *)malloc_bucket(tid, kCacheLineSize);
        if (!p)
            goto MEM_FAIL;
        pchains = (uint64_t)virt2phy(p);
        *p = kCacheLineSize;
        ++ p;
        goto WRITE_AND_EXIT;
    }

    chains_size =  *p;
    ++ p;
    if (nr_items * sizeof(uint64_t) == chains_size  - sizeof(uint64_t)) {
        chains_size += kCacheLineSize;
        uint64_t *pp = p;
        p = (uint64_t *)malloc_bucket(tid, chains_size);
        if (!p)
            goto MEM_FAIL;
        pchains = virt2phy(p);
        *p = kCacheLineSize;
        ++ p;
        memcpy(pp, p, nr_items * sizeof(uint64_t));
        // TODO: FreeBucket(pp - 1);
    }

    for (uint64_t i = 0; i < nr_items; ++ i) {
        uint64_t tag = highbits(16, *p);
        if (tag == key.HighHash16()) {

            uint64_t phy = lowbits(kAddrShift, *p);
            const char *pp = (decltype(pp)) phy2virt(phy);
            auto pkey = Pair(pp);
            
            if (pkey.second == key.size() &&
                    !memcmp(pkey.first, key.data(), key.size())) {
                FreeKVItem((char *)pkey.first);
                goto WRITE_AND_EXIT;
            }
        }
        ++ p;
    }

WRITE_AND_EXIT:
    *p = (key.HighHash16() << 48) | virt2phy(key.data() - 8);
    *entry = pchains | (nr_items << kNItemsShift);
    return *entry;

MEM_FAIL:
    return 0;

#else
        //
    uint64_t old_entry = *entry;
    uint64_t new_entry;
    clear_bit(63, &old_entry);

    uint64_t nr_items = highbits(8, old_entry); 
    uint64_t phy = 0;
    
   persist_key(key); 

    //printf("ADD %lu %p\n", nr_items, key.data() - 8);
    // 1. empty bucket
    if (nr_items == 0) {
        
        phy = virt2phy(key.data() - 8);
        new_entry = phy | (1ul << kNItemsShift);
        //Pair(phy2virt(phy));

        *entry = new_entry;
        clflush(entry, sizeof(*entry));
        return  new_entry;
    }

    uint64_t old_pos = lowbits(48, old_entry);
    uint64_t *old_virt = (decltype(old_virt))phy2virt(old_pos);
    uint64_t old_bucket_size = *old_virt;

    uint64_t new_pos = old_pos;
    uint64_t *virt = old_virt + 1;

    int cpuid = sched_getcpu();
    if (cpuid == -1) {
        perror("get cpuid:");
        exit(-1);
    }
    

    // 2. there is only one item 
    if (nr_items == 1) {
        auto old_key = Pair((const char *)old_virt);
        if (old_key.second == key.size() &&
                !memcmp(key.data(), old_key.first, key.size())) {
            //free old key
            FreeKVItem((char *)old_key.first);

            phy = virt2phy(key.data() - 8);
            //Pair(phy2virt(phy));
            new_entry = phy | (1ul << kNItemsShift);
        } else {
            virt = (uint64_t *)malloc_bucket(cpuid, kCacheLineSize);
            if (virt == (uint64_t *)0)
                return 0;
            new_pos = virt2phy((const char *)virt);
            *virt = kCacheLineSize;
            ++ virt;

            assert(old_key.second == 8);

            uint64_t hash = util::Hash64(old_key.first, old_key.second);
            phy = virt2phy(old_key.first) - 8;
            *virt = (highbits(16, hash) << 48) | phy;

            ++ virt;
            phy = virt2phy(key.data() - 8);
            //Pair(phy2virt(phy));
            *virt = (key.HighHash16() << 48) | phy;

            clflush(virt - 3, kCacheLineSize);
            new_entry = new_pos | (2ul << kNItemsShift);
        }
        *entry = new_entry;
        clflush(entry, sizeof(*entry));
        return  new_entry;
    }

    // 3. there are many items
    // 3.1 make sure the bucket is big enough
    if ((nr_items + 1) * sizeof(uint64_t) >= old_bucket_size) {
            uint64_t new_bucket_size = old_bucket_size + kCacheLineSize;
            virt = (uint64_t *)malloc_bucket(cpuid, new_bucket_size);
            if (virt == (uint64_t *) 0)
                return 0;
            new_pos = virt2phy((const char *)virt);
            memcpy(virt, old_virt, old_bucket_size);
            *virt = new_bucket_size;

            clflush(virt, new_bucket_size);

            ++ virt;
            free(old_virt, old_bucket_size);
    }
    

    // 3.2 try to match with existing key
    // nr_items >= 2
    for (size_t i = 0; i < nr_items; ++ i) {
        uint64_t tag = highbits(16, *virt);
        if (tag == key.HighHash16()) {

            uint64_t phy = lowbits(kAddrShift, *virt);
            const char *loc = phy2virt(phy);
            auto old_key = Pair(loc);
            
            if (old_key.second == key.size() &&
                    !memcmp(old_key.first, key.data(), key.size())) {
                /*
                if (*(uint64_t *)(old_key.first - 16) > key.Version()) {
                    FreeKVItem((void *)key.data());
                    *entry = old_entry;
                    return *entry;
                }
                */

                phy = virt2phy(key.data() - 8);
                //Pair(phy2virt(phy));
                *virt = (tag << 48) | virt2phy(key.data() - 8);
                FreeKVItem((char *)old_key.first);

                clear_bit(63, entry);
                clflush(virt, sizeof(*virt));
                clflush(entry, sizeof(*entry));
                return  *entry;
            }
        }
        ++ virt;
    }

    // 3.3 it is a new key
    ++ nr_items;
    phy = virt2phy(key.data() - 8);
    //Pair(phy2virt(phy));
    *virt = (key.HighHash16() << 48) | virt2phy(key.data() - 8);
    new_entry = new_pos | (nr_items << kNItemsShift);

    *entry = new_entry;
            
    clflush(virt, sizeof(*virt));
    clflush(entry, sizeof(*entry));
    return  *entry;
#endif
}


// @return offset in memory
// Format of an item:
//      length[buff]|
//      version     |
//      length[key] | key | [8-byte aligned] |
//      length[value| value | [8-byte aligned] |
//      checksum[buffer]
//
//8-byte aligned!
//
const char * DataZone::WriteKVItem(const char *key, uint64_t key_size,  const char *value, uint64_t value_size, uint64_t version)
{
    //static int slot = 0;
    //slot = (slot + 1) % kNrLogs;
    int slot = sched_getcpu();
    if (slot == -1)
        slot = 0;
    uint64_t data_size = aligned8(key_size) + aligned8(value_size) 
        + sizeof(struct kv_layout);
    assert(sizeof(struct kv_layout) ==  40);

    data_size = aligned8(data_size); 
    char *virt = (decltype(virt))malloc_generic(slot, data_size);
    if (!virt)
        return nullptr;
    struct kv_layout *playout = (decltype(playout))virt;

    memcpy(virt, &data_size, sizeof(playout->buff_size));  
    virt += sizeof(data_size);

    memcpy(virt, &version, sizeof(playout->version));  
    virt += sizeof(version);;

    memcpy(virt, &key_size, sizeof(playout->key_size));  
    memcpy(virt + 8, key, key_size);
    virt += (8 + aligned8(key_size));

    memcpy(virt, &value_size, sizeof(playout->value_size));  
    memcpy(virt + 8, value, value_size);
    virt += (8 + aligned8(value_size));

    uint64_t checksum = util::Hash64((char *)playout, playout->buff_size - 8);
    memcpy(virt, &checksum, sizeof(playout->checksum));

    return playout->key;
}

pair<const char*, uint64_t>  DataZone::Pair(const char *pos)
{
    uint64_t aligned = (uint64_t)pos;
    aligned = ((aligned + 7) >> 3) << 3;
    uint64_t length = *(uint64_t *)aligned;
#ifdef MYDEBUG
    assert(length == 8 || length == 100);
#endif
    return {(char *)aligned + 8, length};
}

// @addr: offset of key-value
//        | lkey | key | lvalue | value |
pair<const char *, uint64_t> DataZone::GetKVItem(const char *pos, const Key &key)
{
    auto pkey = Pair(pos);
    if (pkey.second == key.size() &&
            !memcmp(key.data(), pkey.first, key.size())) {

        pos += (8 + key.size()); 
        return Pair(pos);
    }

    return {nullptr, 0};
}

pair<const char *, uint64_t> DataZone::GetValue(const Key &key) 
{
    char *virt =  phy2virt(key.ChainsAddr());
    uint64_t nr_items = key.ItemsOfSameHash();
    pair<const char *, uint64_t> null{nullptr, 0};


    //printf("GetVaule: nr_items %d physic %lX \n", nr_items, addr);
    if (nr_items == 0)
        return null;

    if (nr_items == 1) 
        return GetKVItem(virt, key);

    // probe in index-
    // each index is a pair of <hash(16), addr(48)>
    
    pair<const char *, uint64_t> pvalue(null);

    virt += 8;
    for (size_t i = 0; i < nr_items; ++ i) {
        //printf("this %p i %lu addr %p keychains 0x%lx\n",this, i, virt, key.ChainsAddr());
        uint64_t index_data = *(uint64_t *)virt;
        if (key.HighHash16() == highbits(16, index_data)) {

            const char *pos = phy2virt(lowbits(48, index_data));
            pvalue = GetKVItem(pos, key);
            if (pvalue != null)
                return pvalue;
        }
        virt += sizeof(uint64_t);
    }
    return null;
}


