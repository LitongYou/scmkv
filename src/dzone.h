#ifndef _DZONE_H
#define _DZONE_H

#include "zone.h"
#include "key.h"
#include "allocater.h"
#include <cstdint>
#include <thread>

class DataZone: private Zone {
    public:
        DataZone(Zone &z,Allocater **al):Zone(z), _al(al){}

        const char *WriteKVItem(const char *key, uint64_t key_size,  const char *value, uint64_t value_size, uint64_t version);
        // return location in bucket if success, otherwise return -1;
        pair<const char *, uint64_t> GetKVItem(const char *pos, const Key &key);
        pair<const char *, uint64_t> GetValue(const Key &key);
        pair<const char*, uint64_t>  Pair(const char *pos);

        uint64_t WriteIndirectIndex(const string &data);
        uint64_t AddEntryInHTable(uint64_t *entry, const Key &key);
        uint64_t DelEntryInHTable(uint64_t *entry, const Key &key);


        void FreeKVItem(const char *pkey);
    private:
        Allocater **_al;

        Key GetKey(uint64_t physic);

        uint64_t malloc_generic(int slot, uint32_t size) 
        {
            uint64_t vaddr;
            if ((vaddr = (*_al)->malloc_generic(slot, size)) == 0)
                printf("allocate fail\n");
            return vaddr;
        }

        uint64_t malloc_bucket(int slot, uint32_t size) 
        {
            uint64_t vaddr;
            
            if ((vaddr = (*_al)->malloc_bucket(slot, size)) == 0)
                printf("allocate fail\n");
            return vaddr;
        }

        //free a space
        void free(void *virt,uint64_t size) 
        {
            (*_al)->free(virt, size);
        }
};

#endif

