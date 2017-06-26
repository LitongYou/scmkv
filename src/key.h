#ifndef _KEY_H
#define _KEY_H

#include <string>
#include <cstdint>
#include "bitops.h"
//#include "hfunc.h"
#include "hash/farmhash.h"
#include "config.h"

using namespace std;

    
// All pointers are virtual address
struct Key {
    public:
        //friend bool operator==(const Key &key1, const Key &key2);

        enum Operation {
            kKeyNew = 0,
            kKeyDelete = 1,
        };

        Key(const char *key = nullptr, int key_size = 0, uint64_t version = 0,
                uint64_t chains = 0, Operation _op = kKeyNew):
            _data(key), data_size(key_size),
            _version(version),
            op(_op) ,chains(chains)
        {
            hash = util::Hash64(key, key_size);
        }

        int ItemsOfSameHash() const { return highbits(8, chains); };

        uint64_t ChainsAddr() const { return lowbits(48, chains);}
        void SetChains(uint64_t pchains) {
            chains = pchains;
            clear_bit(63, &chains);
        }

        // set addr[0..63]
        void SetKey(const char* virt) { 
            // reset pointer to key;
            _data = virt;
        }

        uint64_t Version() const { return _version; }

        uint64_t HighHash16() const { return highbits(16, hash);}
        uint64_t HighHash32() const { return highbits(32, hash);}
        uint64_t Hash64() const { return hash; }
        uint64_t size() const { return data_size; }
        const char *data() const { return _data;}
        Operation Op() const { return op;}
        uint64_t data_phy() const; 

        //const string data; // orignal key
   private:
        const char *_data;
        int data_size;
        uint64_t _version;

        Operation op;
        uint64_t hash;
        /* Format of addr
         * [ 0 ... 47 | 48 ... 55 | 56 ... 62 | 63 |
         *   offset      reserve     nr_items  lock
         */ 
        uint64_t chains;
};

inline bool operator==(const Key &key1, const Key &key2) {
    return key1.Op() == key2.Op() &&
        key1.Hash64() == key2.Hash64() &&
        key1.size() == key2.size() &&
        memcmp(key1.data(), key2.data(), key1.size()) == 0;
}
#endif
