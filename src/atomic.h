#ifndef _ATOMIC_H
#define _ATOMIC_H

#include <cstdint>

template<typename DestType, typename SrcType>
inline bool atomic_compare_exchange(DestType *dest, DestType *expected, SrcType value)
{
    static_assert(sizeof(DestType) == 8 || 
            sizeof(DestType) == 4,
            "Atomic only work on 4- and 8-byte values!");

    DestType old = *expected;

    if (sizeof(DestType) == 8) {
        asm volatile("lock cmpxchgq %2, %1\n"
                :"=a"(*expected), "+m"(*dest)
                :"r"(value), "0"(*expected)
                );

        return (uint64_t)old == (uint64_t)*expected;
    } else if (sizeof(DestType) == 4) {
        asm volatile("lock cmpxchgl %2, %1"
                :"=a"(*expected), "+m"(*dest)
                :"r"(value), "0"(*expected)
                );

        return (uint32_t)old == (uint32_t)*expected;
    }

    return false;
}


template<typename T>
inline void atomic_set(T *t, T v) 
{
    if (sizeof(T) == 8) {
        *(volatile uint64_t *)t = (uint64_t)v;
    }
}

#endif
