#ifndef __TELEIOS_PROFILER__
#define __TELEIOS_PROFILER__

#include "teleios/defines.h"
#include "teleios/logger.h"

void tl_profiler_frame_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);
void tl_profiler_frame_pop();

#define TL_PROFILER_PUSH_WITH(args, ...)                                              \
    {                                                                                 \
        tl_profiler_frame_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);    \
        TLVERBOSE("[in ] %s(...)", __func__)                                          \
    }

#define TL_PROFILER_PUSH                                              \
    {                                                                 \
        tl_profiler_frame_push(__FILE__, __LINE__, __func__, NULL);   \
        TLVERBOSE("[in ] %s(...)", __func__)                          \
    }

#define TL_PROFILER_POP                         \
    do {                                        \
        tl_profiler_frame_pop();                \
        TLVERBOSE("[out] %s(...)", __func__)    \
        return;                                 \
    } while (false);

#define TL_PROFILER_POP_WITH(v)                 \
    do {                                        \
        tl_profiler_frame_pop();                \
        TLVERBOSE("[out] %s(...)", __func__)    \
        return v;                               \
    } while (false);

#endif