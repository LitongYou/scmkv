#ifndef _LOG_H
#define _LOG_H

#include <cstdio>
#include <cstdarg>

inline void loger(const char *format, ...)
{
    return ;

    va_list args;
    va_start(args, format);
    //fprintf(stderr, "%s-%s#%d: ", __FILE__, __func__, __LINE__);
    vfprintf(stderr, format, args);
    va_end(args);
}

#endif
