#ifndef _DB_H
#define _DB_H

#include <cstdint>
#include <string>
#include <thread>
#include <queue>
#include <list>
#include <mutex>
#include <atomic>

#include "device.h"
#include "memlayout.h"
#include "key.h"
#include "log.h"
#include "utils.h"

using namespace std;

struct db_config {
    bool force_create;
};

static const db_config default_db_config = {false};

class DB {
    public: 
        DB(Device dev, db_config cf = default_db_config):
            _mempool(kBufferSize),
            _quit(false), _mstore(dev), _config(cf) {
            _init();
        }

        ~DB() {
            _quit = true;
            for(int i = 0; i < kNumOfThreads; ++ i) {
                _merge_threads[i]->join();
                delete _merge_threads[i];
            }
            printf("destroy DB\n");
        }

        int Put(const char *key, int key_size,const char *value, int value_size);
        pair<const char*, int> Get(const char *key, int key_size);
        int Delete(const char *key, int key_size);

    private:
        void _init () {
            for(int i = 0; i < kNrLogs; ++ i) {
                for (int j = 0; j < kSizeOfHCache; ++ j) {
                    _hcache[i][j].set_buffer(&_mempool);
                }
            }
            _merge_threads[0] = new thread(&DB::Merge, this, 0);

            if (_config.force_create || !_mstore.Recovery()) {
                _mstore.Create();
                _mstore.Recovery();
            }
            _version = _mstore.Version();
        }

        void Commit(const Key &key);
        void Merge(int id);

        static const int kNumOfThreads = kMaxConcurrent;
        static const int kSizeOfHCache = 1 << 10;
        static const size_t kBufferSize = 1 << 20;

        using list_t = buffered_list<Key>;
        using node_t = list_t::node_type;
        using queue_t = circular_queue<Key *, kBufferSize>;

        mutex _mtx[kNumOfThreads];
        buffer<node_t> _mempool;
        queue_t _que[kNrLogs];
        list_t  _hcache[kNrLogs][kSizeOfHCache];

        thread *_merge_threads[kNumOfThreads];

        bool _quit;

        MemoryStorage _mstore;

        db_config _config;

        atomic<uint64_t> _version;
};

#endif
