/**
 * @file profiler.h
 * @brief Stack-based function call profiler for call hierarchy tracing and debugging
 *
 * This module provides a lightweight call stack profiler for understanding program
 * execution flow and identifying performance bottlenecks. The profiler is stack-based,
 * meaning it tracks nested function calls in a call hierarchy.
 *
 * Key features:
 * - Stack-based call tracking (up to 1000 nested levels)
 * - Automatic filename, function name, and line number capture
 * - Optional argument logging for parameter values
 * - Entry/exit logging through the logger subsystem
 * - Zero overhead in Release builds (completely compiled out)
 * - Thread-local storage for thread-safe operation
 */

#ifndef __TELEIOS_PROFILER__
#define __TELEIOS_PROFILER__

#include "teleios/defines.h"
#include "teleios/logger.h"
#include <stdarg.h>

#if ! defined(TELEIOS_FRAME_MAXIMUM)
#   define TELEIOS_FRAME_MAXIMUM 1000
#endif

#if ! defined(TL_PROFILER_FRAME_ARGUMENTS_SIZE)
#   define TL_PROFILER_FRAME_ARGUMENTS_SIZE 1024
#endif

typedef struct {
    const char* filename;
    const char* function;
    char arguments[TL_PROFILER_FRAME_ARGUMENTS_SIZE];
    u32 lineno;
} TLFrame;

// Thread-local profiler state
static TL_THREADLOCAL u16 tl_profiler_frame_index = U16_MAX;
static TL_THREADLOCAL TLFrame tl_profiler_frames[TELEIOS_FRAME_MAXIMUM];

#if defined(TELEIOS_BUILD_DEBUG)

/**
 * @brief Push new function frame to call stack and log entry
 */
inline void tl_profiler_frame_push(const char* filename, const u32 lineno, const char* function, const char* arguments, ...) {
    tl_profiler_frame_index++;

    if (TL_UNLIKELY(tl_profiler_frame_index >= TELEIOS_FRAME_MAXIMUM)) {
        tl_logger_write(TL_LOG_LEVEL_FATAL, filename, lineno, "TELEIOS_FRAME_MAXIMUM of %d exceeded", TELEIOS_FRAME_MAXIMUM);
        exit(99);
    }

    tl_profiler_frames[tl_profiler_frame_index].filename = filename;
    tl_profiler_frames[tl_profiler_frame_index].function = function;
    tl_profiler_frames[tl_profiler_frame_index].lineno = lineno;

    if (TL_LIKELY(arguments == NULL)) {
        tl_profiler_frames[tl_profiler_frame_index].arguments[0] = '\0';
    } else {
        va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(tl_profiler_frames[tl_profiler_frame_index].arguments, TL_PROFILER_FRAME_ARGUMENTS_SIZE, arguments, arg_ptr);
        va_end(arg_ptr);
    }

    tl_logger_write(TL_LOG_LEVEL_VERBOSE, filename, lineno, "[in ] %s(%s)",
                    function, tl_profiler_frames[tl_profiler_frame_index].arguments);
}

/**
 * @brief Pop current frame from call stack and log exit
 */
inline void tl_profiler_frame_pop(void) {
    tl_logger_write(TL_LOG_LEVEL_VERBOSE,
                    tl_profiler_frames[tl_profiler_frame_index].filename,
                    tl_profiler_frames[tl_profiler_frame_index].lineno,
                    "[out] %s(%s)",
                    tl_profiler_frames[tl_profiler_frame_index].function,
                    tl_profiler_frames[tl_profiler_frame_index].arguments
    );
    memset(&tl_profiler_frames[tl_profiler_frame_index], 0, sizeof(TLFrame));
    tl_profiler_frame_index--;
}

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