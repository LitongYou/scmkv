#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

#define BUCKET_SHIFT 29
#define NR_BUCKET  (1ul << BUCKET_SHIFT)
#define BUCKET_SIZE 8

#define NO_BUCKET(hash) ((hash) & (NR_BUCKET - 1))
#define BUCKET_ADDR(start, key, length) \
    ((uint64_t *)(start) + NO_BUCKET(hash(key, length)))
#define SET_BUCKET(addr, ptr) *addr = (uint64_t) ptr

const int kNItemsShift = 56;
const int kAddrShift = 48;
const uint64_t kAddrMask = (1ul << kAddrShift)  - 1;


const int kPageSize =  (2ul << 20);
const int kCacheLineSize = 64;
const int kBucketSize = sizeof(uint64_t);
const int kDataAlign = 8;

const int kMaxConcurrent = 1;
const int kNrLogs = 8;

#endif

