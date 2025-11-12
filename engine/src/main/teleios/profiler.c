#include "teleios/teleios.h"

#if ! defined(TELEIOS_FRAME_MAXIMUM)
#   define TELEIOS_FRAME_MAXIMUM 1000
#endif

#if ! defined(TL_PROFILER_FRAME_ARGUMENTS_SIZE)
#   define TL_PROFILER_FRAME_ARGUMENTS_SIZE 1024
#endif

typedef struct {
    const char* function;
    const char* filename;
    char arguments[TL_PROFILER_FRAME_ARGUMENTS_SIZE];
    u32 lineno;
} TLFrame;

// Thread-local profiler state
static u16 tl_profiler_frame_index = U16_MAX;
static TLFrame tl_profiler_frames[TELEIOS_FRAME_MAXIMUM];

/**
 * @brief Push new function frame to call stack and log entry
 */
void tl_profiler_frame_push(const char* filename, const u32 lineno, const char* function, const char* arguments, ...) {
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
void tl_profiler_frame_pop(void) {
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