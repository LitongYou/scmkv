#ifndef _HTABLE_H
#define _HTABLE_H

#include <openssl/md5.h>
#include "zone.h"
#include "key.h"
#include "log.h"
#include "config.h"

class HashTable: private Zone {
    public:
        HashTable(Zone &z):Zone(z), kBucketMask(_size / kBucketSize - 1){
            loger("HashTable initial: mask 0x%X\n", kBucketMask);
        }
        uint64_t Set(const Key &key);
        uint64_t Unset(const Key &key);
        uint64_t Get(Key &key);

    private:
        uint64_t BucketIndex(uint64_t hash);
        const int kBucketMask;

};


#endif
