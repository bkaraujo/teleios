#ifndef __TELEIOS_META__
#define __TELEIOS_META__

#include "teleios/defines.h"
#include "teleios/core/profiler.h"

void tl_meta_frame_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);
void tl_meta_frame_pop();

#define TLSTACKPUSHA(args, ...)                                                 \
    {                                                                           \
        /* Operation order matters */                                           \
        tl_meta_frame_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);  \
        tl_profiler_begin(__func__);                                            \
    }

#define TLSTACKPUSH                                             \
    {                                                           \
        /* Operation order matters */                           \
        tl_meta_frame_push(__FILE__, __LINE__, __func__, NULL); \
        tl_profiler_begin(__func__);                            \
    }

#define TLSTACKPOP                      \
    do {                                \
        /* Operation order matters */   \
        tl_profiler_end(__func__);      \
        tl_meta_frame_pop();            \
        return;                         \
    } while (FALSE);

#define TLSTACKPOPV(v)                  \
    do {                                \
        /* Operation order matters */   \
        tl_profiler_end(__func__);      \
        tl_meta_frame_pop();            \
        return v;                       \
    } while (FALSE);

#endif //__TELEIOS_META__
