#ifndef __TELEIOS_PROFILER__
#define __TELEIOS_PROFILER__

#include "teleios/defines.h"

/**
 * @brief Push new function frame to call stack and log entry
 */
void tl_profiler_frame_push(const char* filename, u32 lineno, const char* function, const char* arguments, ...) ;

/**
 * @brief Pop current frame from call stack and log exit
 */
void tl_profiler_frame_pop(void);

#if defined(TELEIOS_BUILD_DEBUG)
#   define TL_PROFILER_PUSH { tl_profiler_frame_push(__FILE__, __LINE__, __func__, NULL); }
#   define TL_PROFILER_PUSH_WITH(args, ...) { tl_profiler_frame_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__); }
#   define TL_PROFILER_POP { do { tl_profiler_frame_pop(); return; } while (false); }
#   define TL_PROFILER_POP_WITH(v) { do { tl_profiler_frame_pop(); return v; } while (false); }

#else
#   define TL_PROFILER_PUSH
#   define TL_PROFILER_PUSH_WITH(args, ...)
#   define TL_PROFILER_POP { return; }
#   define TL_PROFILER_POP_WITH(v) { return v; }
#endif

#endif