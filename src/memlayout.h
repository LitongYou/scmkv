#ifndef _MEMLAYOUT_H
#define _MEMLAYOUT_H

#include "device.h"
#include "super.h"
#include "page.h"
#include "htable.h"
#include "dzone.h"
#include "allocater.h"

#include <cassert>

using namespace std;

const uint64_t kMagicSize = 8;

class MemoryStorage {
    public:
        MemoryStorage(Device &dev):_dev(dev){};

        PageTable   *pt;
        HashTable   *ht;
        DataZone    *dz;
        Allocater   *allocater;

        static uint64_t VirtBase() { 
            assert(_virt_base % 8 == 0);
            return _virt_base; 
        }
            
        static MemoryStorage *Mstore() { return _mstore;}

        bool Recovery();
        bool Create();

        uint64_t Version() { return _version; }

    private:
        Device _dev;
        static uint64_t _virt_base;
        uint64_t _size;
        int _nr_merger;
        uint64_t _version;

        static MemoryStorage *_mstore; 
        static const char kMagicString[8];
};

inline char* phy2virt(uint64_t phy) {
    return (char *)MemoryStorage::VirtBase() + phy;
}

inline uint64_t virt2phy(const void *virt) {
    return (uint64_t)virt - MemoryStorage::VirtBase();
}
#endif
