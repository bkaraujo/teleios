#ifndef __TELEIOS_CORE_META__
#define __TELEIOS_CORE_META__

#include "teleios/defines.h"

void tl_profiler_begin(const char *name);
void tl_profiler_end(const char *name);
void tl_profiler_tick(const char *name);
u64 tl_profiler_time(const char *name);
u64 tl_profiler_ticks(const char *name);

#define TL_PROFILER_TICK   tl_profiler_tick(__func__)
#define TL_PROFILER_TICKS  tl_profiler_ticks(__func__)
#define TL_PROFILER_MICROS tl_profiler_time(__func__)

void tl_trace_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);
void tl_trace_pop();

#define TL_STACK_PUSHA(args, ...)                                                 \
    {                                                                           \
        /* Operation order matters */                                           \
        tl_trace_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);       \
        tl_profiler_begin(__func__);                                            \
    }

#define TL_STACK_PUSH                                             \
    {                                                           \
        /* Operation order matters */                           \
        tl_trace_push(__FILE__, __LINE__, __func__, NULL);      \
        tl_profiler_begin(__func__);                            \
    }

#define TL_STACK_POP                      \
    do {                                \
        /* Operation order matters */   \
        tl_profiler_end(__func__);      \
        tl_trace_pop();                 \
        return;                         \
    } while (false);

#define TL_STACK_POPV(v)                  \
    do {                                \
        /* Operation order matters */   \
        tl_profiler_end(__func__);      \
        tl_trace_pop();                 \
        return v;                       \
    } while (false);

#endif //__TELEIOS_CORE_META__
