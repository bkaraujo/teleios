#include "teleios/teleios.h"
#include "profiler/types.inl"

// Thread-local profiler state
static TL_THREADLOCAL u16 tl_profiler_frame_index = U16_MAX;
static TL_THREADLOCAL TLStackFrame tl_profiler_frames[TELEIOS_FRAME_MAXIMUM];

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

    // tl_logger_write(TL_LOG_LEVEL_VERBOSE, filename, lineno, "[in ] %s(%s)",
    //                 function, tl_profiler_frames[tl_profiler_frame_index].arguments);
}

/**
 * @brief Pop current frame from call stack and log exit
 */
void tl_profiler_frame_pop(void) {
    // tl_logger_write(TL_LOG_LEVEL_VERBOSE,
    //                 tl_profiler_frames[tl_profiler_frame_index].filename,
    //                 tl_profiler_frames[tl_profiler_frame_index].lineno,
    //                 "[out] %s(%s)",
    //                 tl_profiler_frames[tl_profiler_frame_index].function,
    //                 tl_profiler_frames[tl_profiler_frame_index].arguments
    // );
    memset(&tl_profiler_frames[tl_profiler_frame_index], 0, sizeof(TLStackFrame));
    tl_profiler_frame_index--;
}

/**
 * @brief Capture current call stack
 */
void tl_profiler_stacktrace_snapshot(TLStackTrace* trace) {
    if (trace == NULL) {
        return;
    }

    memset(trace, 0, sizeof(TLStackTrace));

    // Capture up to TL_STACKTRACE_MAX_DEPTH frames from the current call stack
    const u16 max_frames = (tl_profiler_frame_index + 1 < TELEIOS_FRAME_MAXIMUM) ? tl_profiler_frame_index + 1 : TELEIOS_FRAME_MAXIMUM;

    for (u16 i = 0; i < max_frames; i++) {
        const TLStackFrame* source = &tl_profiler_frames[i];
        TLStackFrame* dest = &trace->frames[i];

        dest->filename = source->filename;
        dest->function = source->function;
        dest->lineno = source->lineno;

        // Copy arguments
        memcpy(dest->arguments, source->arguments, TL_PROFILER_FRAME_ARGUMENTS_SIZE);
    }

    trace->depth = max_frames;
}

/**
 * @brief Print a captured stack trace to the logger
 */
void tl_profiler_stacktrace_print(const TLStackTrace* trace) {
    if (trace == NULL || trace->depth == 0) {
        tl_logger_write(TL_LOG_LEVEL_WARN, __FILE__, __LINE__, "  (no stack trace available)");
        return;
    }

    tl_logger_write(TL_LOG_LEVEL_WARN, __FILE__, __LINE__, "  Stack trace (%u frames):", trace->depth);

    for (u16 i = 0; i < trace->depth; i++) {
        const TLStackFrame* frame = &trace->frames[i];

        // Extract just the filename (not full path)
        const char* filename = strrchr(frame->filename, tl_filesystem_path_separator());
        filename = (filename == NULL) ? frame->filename : filename + 1;

        if (frame->arguments[0] != '\0') {
            tl_logger_write(TL_LOG_LEVEL_WARN, __FILE__, __LINE__, "    #%02d: %20s:%04d %s(%s)", i, filename, frame->lineno, frame->function, frame->arguments);
        } else {
            tl_logger_write(TL_LOG_LEVEL_WARN, __FILE__, __LINE__, "    #%02d: %20s:%04d %s", i, filename, frame->lineno, frame->function);
        }
    }
}