#include <cstdio>
#include <cstdint>
#include <functional>
#include <future>
#include <vector>
#include <iomanip>
#include <unistd.h>
#include <pthread.h>
#include "db.h"
#include "gen.h"


using namespace std;
using namespace std::placeholders;

#define SCMKV_TEST

struct worker_config {
    uint64_t klen;
    uint64_t vlen;
    uint64_t reqs;
    char ktype;
    char etype; //evaluation type
};

static void set_thread_affinity(uint64_t cpu)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
}

Generator<uint64_t> * MakeGenerator(const worker_config &cfg) {
    switch (cfg.ktype) {
        case 'z':
            return  new ZipfianGenerator(9999999900000000ul, 9999999999999999ul);
        case 'u':
        default:
            return new UniformGenerator<uint64_t>(9999999999999999ul);
    }

}

void to_number(char *dst, int len, uint64_t value) {
    memset(dst, '0', len);
    while (value > 0) {
        *(dst ++) = value % 10 + '0';
        value /= 10;
    }
}

void ScmkvWorker(DB &db, int tid, const worker_config &cfg, uint64_t &result) {
    set_thread_affinity(tid - 1);
    char *pk = new char[cfg.klen + 1]{0};
    char *pv = new char[cfg.vlen + 1];
    //string kformat = "\"%0" + to_string(cfg.klen) + "lu\"";

    Generator<uint64_t> *gi = MakeGenerator(cfg);

    auto writedb = [&]() {
        for (size_t i = 0; i < cfg.reqs; ++ i) {
            to_number(pk, cfg.klen, gi->next()); 
            db.Put(pk, cfg.klen, pv, cfg.vlen);
        }
    };
    auto readdb = [&]() {
        for (size_t i = 0; i < cfg.reqs; ++ i) {
            to_number(pk, cfg.klen, gi->next()); 
            db.Get(pk, cfg.klen);
        }
    };

    if (cfg.etype == 'r') {
        writedb();
    }

    auto start = chrono::system_clock::now();
    
    if (cfg.etype == 'r') {
        readdb();
    } else if (cfg.etype == 'w') {
        writedb();
    }

    chrono::duration<double> dur = chrono::system_clock::now() - start;
    delete pk;
    delete pv;
    delete gi;
    result = cfg.reqs / dur.count() / 1000;
    printf("thread %d run %fs Kops %lu\n", tid, dur.count(), result);
}

template<typename put_f, typename get_f>
class Worker {
    public:
        Worker(put_f put, get_f get): 
            _put(put), _get(get) { }
        void operator() (int tid, const worker_config &cfg, uint64_t & result);

    private:
        put_f _put;
        get_f _get;
};


template<typename put_f, typename get_f>
void Worker<put_f, get_f>::operator() (int tid, const worker_config &cfg, uint64_t &result) {

    char *pk = new char[cfg.klen];
    char *pv = new char[cfg.vlen];
    string kformat = "\"%0" + to_string(cfg.klen) + "lu\"";
    printf("kformat %s\n", kformat.c_str());

    //Generator<uint64_t> *gi = new UniformGenerator<uint64_t>(999999999ul);
    Generator<uint64_t> *gi = MakeGenerator(cfg); 

    auto start = chrono::system_clock::now();
    
    for (size_t i = 0; i < cfg.reqs; ++ i) {
        sprintf(pk,  kformat.c_str(), gi->next());
        _put(pk, cfg.klen, pv, cfg.vlen);
    }

    chrono::duration<double> dur = chrono::system_clock::now() - start;
    delete pk;
    delete pv;
    delete gi;
    result = cfg.reqs / dur.count() / 1000;
    printf("thread %d run %fs Kops %lu\n", tid, dur.count(), result);
    //return result;
}



int main(int argc, char *argv[]) {
    int opt;
    worker_config cfg = {16, 100, 1, 'u', 'w'};
    int nthread = 1;
    while ((opt = getopt(argc, argv,
                    "k:" // constant value size
                    "v:" // constant value size
                    "c:"  // reqs
                    "g:" // generator c,e,z,x,u
                    "a:" // nr_threads
                    "e:"
                    )) != -1) {
        switch(opt) {
            case 'k': cfg.klen       = strtoull(optarg, NULL, 10); break;
            case 'v': cfg.vlen       = strtoull(optarg, NULL, 10); break;
            case 'c': cfg.reqs       = strtoull(optarg, NULL, 10); break;
            case 'g': cfg.ktype      = optarg[0]; break;
            case 'e': cfg.etype      = optarg[0]; break;
            case 'a': nthread = strtoull(optarg, NULL, 10); break;
            case '?': perror("arg error\n"); exit(1);
            default : perror("arg error\n"); exit(1);
        }
    }
    cfg.reqs = cfg.reqs * 1000000 / nthread;

#ifdef SCMKV_TEST
    VirtDevice dev(4ul << 30);
    dev.Open();
    db_config cf = {true};
    DB db(dev, cf);
    auto put = bind(&DB::Put, &db, _1, _2, _3, _4);
    auto get = bind(&DB::Get, &db, _1, _2);
#endif

    uint64_t  Kops_sum = 0;
    vector<uint64_t> Kops_per_thread(nthread, 0);
    vector<thread> thds(nthread);
    for (int i = 0; i < nthread; ++ i) {
        //thds[i] = thread(Worker<decltype(put), decltype(get)>(put, get), i + 1, cfg, ref(Kops_per_thread[i]));
        thds[i] = thread(&ScmkvWorker, ref(db), i + 1, ref(cfg), ref(Kops_per_thread[i]));
    }
    for (int i = 0; i < nthread; ++ i) {
        thds[i].join();
    }
    for (auto e: Kops_per_thread) {
        Kops_sum += e;
    }

    printf("Kops: %lu\n", Kops_sum);
    return 0;

}
