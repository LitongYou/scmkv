#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include "device.h"
#include "log.h"

#ifndef MAP_HUGE_2MB
#define MAP_HUGE_2MB 0
#endif

int Device::Open() 
{
    _devno = open(_name.c_str(), O_RDWR | O_SYNC);
    if (_devno == -1) {
        perror("device open error");
        return -1;
    }

    printf("%lX %lX\n", _offset, _length);
    loger("device try to mmap with 2M page\n");

   _addr = (uint64_t)mmap(NULL, _length, PROT_READ | PROT_WRITE,
           MAP_SHARED | MAP_NORESERVE | MAP_HUGETLB | MAP_HUGE_2MB, _devno, _offset);

   if (_addr == (uint64_t)-1) {
        loger("device mmap with 4k page\n");
       _addr = (uint64_t)mmap(NULL, _length, PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_NORESERVE, _devno, _offset);
   }

   if (_addr == (uint64_t)-1) {
       perror("device map error");
       close(_devno);
       return -1;
   }

    loger("mmap return %lX\n", _addr);
    return 0;
}

int Device::Close()
{
    printf("close device: %lX %lX\n", _addr, _length);
    munmap((void *)_addr, _length);
    close(_devno);
    return 0;
}

