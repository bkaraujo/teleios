#ifndef __TELEIOS_PROFILER__
#define __TELEIOS_PROFILER__

#include "teleios/defines.h"
#include "teleios/logger.h"

void tl_profiler_frame_pop();
void tl_profiler_frame_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);

#if defined(TELEIOS_BUILD_DEBUG)
#   define TL_PROFILER_PUSH tl_profiler_frame_push(__FILE__, __LINE__, __func__, NULL);
#   define TL_PROFILER_PUSH_WITH(args, ...) tl_profiler_frame_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);
#   define TL_PROFILER_POP do { tl_profiler_frame_pop(); return; } while (false);
#   define TL_PROFILER_POP_WITH(v) do { tl_profiler_frame_pop(); return v; } while (false);
#else
#   define TL_PROFILER_PUSH
#   define TL_PROFILER_PUSH_WITH(args, ...)
#   define TL_PROFILER_POP
#   define TL_PROFILER_POP_WITH(v)
#endif

#endif