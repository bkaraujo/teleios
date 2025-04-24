#ifndef __LIBK_RUNTIME__
#define __LIBK_RUNTIME__

#include "libk/defines.h"

void k_runtime_profiler_push(const char *name);
void k_runtime_profiler_pop(const char *name);
u64 k_runtime_profiler_elapsed(const char *name);

#define K_RUNTIME_PROFILER_ELAPSED k_runtime_profiler_elapsed(__func__)

#if ! defined(K_RUNTIME_FRAME_ARGUMENTS_SIZE)
#   define K_RUNTIME_FRAME_ARGUMENTS_SIZE 1024
#endif

#if ! defined(K_RUNTIME_STRING_SIZE)
#   define K_RUNTIME_STRING_SIZE 100
#endif

void k_runtime_frame_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);
void k_runtime_frame_pop();

#define K_FRAME_PUSH_WITH(args, ...)                                                \
    {                                                                               \
        /* Operation order matters */                                               \
        k_runtime_frame_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);    \
        k_runtime_profiler_push(__func__);                                          \
    }

#define K_FRAME_PUSH                                                \
    {                                                               \
        /* Operation order matters */                               \
        k_runtime_frame_push(__FILE__, __LINE__, __func__, NULL);   \
        k_runtime_profiler_push(__func__);                          \
    }

#define K_FRAME_POP                         \
    do {                                    \
        /* Operation order matters */       \
        k_runtime_profiler_pop(__func__);   \
        k_runtime_frame_pop();              \
        return;                             \
    } while (false);

#define K_FRAME_POP_WITH(v)                 \
    do {                                    \
        /* Operation order matters */       \
        k_runtime_profiler_pop(__func__);   \
        k_runtime_frame_pop();              \
        return v;                           \
    } while (false);


#endif