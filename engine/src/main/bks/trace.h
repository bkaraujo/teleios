#ifndef __BK_STANDARD_TRACE__
#define __BK_STANDARD_TRACE__

#include "bks/defines.h"

void bks_profiler_begin(const char *name);
void bks_profiler_end(const char *name);
void bks_profiler_tick(const char *name);
u64 bks_profiler_time(const char *name);
u64 bks_profiler_ticks(const char *name);

#define BKS_PROFILER_TICK   bks_profiler_tick(__func__)
#define BKS_PROFILER_TICKS  bks_profiler_ticks(__func__)
#define BKS_PROFILER_MICROS bks_profiler_time(__func__)

void bks_trace_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);
void bks_trace_pop();

#define BKS_TRACE_PUSHA(args, ...)                                           \
    {                                                                        \
        /* Operation order matters */                                        \
        bks_trace_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);   \
        bks_profiler_begin(__func__);                                        \
    }

#define BKS_TRACE_PUSH                                       \
    {                                                        \
        /* Operation order matters */                        \
        bks_trace_push(__FILE__, __LINE__, __func__, NULL);  \
        bks_profiler_begin(__func__);                        \
    }

#define BKS_TRACE_POP                 \
    do {                              \
        /* Operation order matters */ \
        bks_profiler_end(__func__);   \
        bks_trace_pop();              \
        return;                       \
    } while (false);

#define BKS_TRACE_POPV(v)             \
    do {                              \
        /* Operation order matters */ \
        bks_profiler_end(__func__);   \
        bks_trace_pop();              \
        return v;                     \
    } while (false);


#endif