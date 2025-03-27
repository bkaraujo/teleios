#ifndef __TELEIOS_CORE_META_TRACE__
#define __TELEIOS_CORE_META_TRACE__

#include "teleios/defines.h"

void tl_trace_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);
void tl_trace_pop();

#define TLSTACKPUSHA(args, ...)                                                 \
    {                                                                           \
        /* Operation order matters */                                           \
        tl_trace_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);       \
        tl_profiler_begin(__func__);                                            \
    }

#define TLSTACKPUSH                                             \
    {                                                           \
        /* Operation order matters */                           \
        tl_trace_push(__FILE__, __LINE__, __func__, NULL);      \
        tl_profiler_begin(__func__);                            \
    }

#define TLSTACKPOP                      \
    do {                                \
        /* Operation order matters */   \
        tl_profiler_end(__func__);      \
        tl_trace_pop();                 \
        return;                         \
    } while (FALSE);

#define TLSTACKPOPV(v)                  \
    do {                                \
        /* Operation order matters */   \
        tl_profiler_end(__func__);      \
        tl_trace_pop();                 \
        return v;                       \
    } while (FALSE);

#endif //__TELEIOS_CORE_META_TRACE__
