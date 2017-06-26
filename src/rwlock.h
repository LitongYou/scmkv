#ifndef _RWLOCK_H
#define _RWLOCK_H

#include <cstdint>
#include <atomic>
#include <cstdio>

using namespace std;

inline void cpu_relax() {
    asm volatile("rep; nop" ::: "memory");
}

class RWLock {
    public:
    
       RWLock(): _lock(kUnLock), _writer(false) {}

       bool RDLock() {
           while (1) {
               if (atomic_fetch_sub(&_lock, 1) >= 0 && !_writer) {
                   //printf("read lock %x\n", _lock.load());
                   return true;
               }
               atomic_fetch_add(&_lock, 1);
           }
           return false;
       }


       bool WRLock() {
           while (1) {
               if (kUnLock == atomic_fetch_sub(&_lock, kUnLock)) {
                   //printf("write lock %x\n", _lock.load());
                   _writer = false;
                   return true; 
               }
               _writer = true;
               atomic_fetch_add(&_lock, kUnLock);
           }
           return false;
       }


       void RDUnLock() {
            atomic_fetch_add(&_lock, 1);
            //printf("read unlock %x\n", _lock.load());
       }

       void WRUnLock() {
            atomic_fetch_add(&_lock, kUnLock);
            //printf("writer unlock %x\n", _lock.load());
       }

    private:
        static const int32_t kUnLock = 0x100000;
        atomic<int32_t> _lock;
        atomic<bool> _writer;
};

#endif
