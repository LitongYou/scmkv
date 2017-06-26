#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <thread>
#include <queue>
#include <fcntl.h>
#include <unistd.h>
#include <sched.h>
#include <atomic>
#include <pthread.h>

#include "config.h"
#include "device.h"
#include "db.h"
#include "key.h"

#include <iostream>

using namespace std;

void DB::Merge(int id) 
{
    pthread_setname_np(pthread_self(), "scmkv-log-cleaner");
    //loger("merge thread %d start\n", id);
    while (!_quit) {
        usleep(1);

        //TODO:
        //log-cleaner
    }
    loger("merge thread %d exit\n", id);
}

void DB::Commit(const Key &key) {
    //int id = key.HighHash32() % kNumOfThreads;
    int slot = key.HighHash32() % kSizeOfHCache;
    int cpuid = sched_getcpu();
    if (cpuid == -1)
        cpuid = 0;

    Key *pkey = nullptr;
    while ((pkey =_hcache[cpuid][slot].push_front(key)) == nullptr)
        usleep(10);
        
    //assert(memcmp(pkey, &key, sizeof(key)) == 0);

    _que[cpuid].push(pkey);
}

int DB::Put(const char *key, int key_size, const char *value, int value_size)
{
    //assert(key_size == 8);
    uint64_t version = atomic_fetch_add(&_version, 1ul);

    const char *pkey = _mstore.dz->
        WriteKVItem(key, key_size, value, value_size, version);

    Key inter_key(pkey, key_size); 

    _mstore.ht->Set(inter_key);
    return 0;

}

static bool compare_key(const Key& d1, const Key &d2) {
    return d1.Hash64() == d2.Hash64() &&
        d1.size() == d2.size() &&
        memcmp(d1.data(), d2.data(), d1.size()) == 0;
}

pair<const char*, int> DB::Get(const char *key, int key_size)
{
    Key inter_key(key, key_size);

    _mstore.ht->Get(inter_key);
    return _mstore.dz->GetValue(inter_key);
}


int DB::Delete(const char *key, int len) {

    Key inter_key(key, len, 0, Key::kKeyDelete);

    _mstore.ht->Get(inter_key);

    auto value = _mstore.dz->GetValue(inter_key);

    if (!value.first) 
        return 0;

    len += 7;
    len = len / 8 * 8;
    value.first -= (len  + 8);

    inter_key.SetKey(value.first);

    _mstore.ht->Unset(inter_key);
    return 0;
}


