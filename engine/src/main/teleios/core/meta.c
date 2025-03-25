#include "teleios/core.h"

#if defined(TLPLATFORM_LINUX)
#include <sys/time.h>
#endif

#if ! defined(TELEIOS_BUILD_RELEASE)
#include <stdio.h>
#include <stdarg.h>
#include "teleios/globals.h"
#endif

void tl_meta_frame_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    if (global->stack_index >= sizeof(global->stack) / sizeof(TLStackFrame)) {
        TLFATAL("global->stack_index exceeded")
    }

    global->stack[global->stack_index].lineno = lineno;

#if defined(TLPLATFORM_LINUX)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    const u64 micros = (uint64_t) tv.tv_sec * 1000000 + tv.tv_usec;

    global->stack[global->stack_index].timestamp =  micros;
#elif defined(TLPLATFORM_WINDOWS)
    FILETIME ft;
    ULARGE_INTEGER uli;
    uint64_t epoch_microseconds;

    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // FILETIME is in 100-nanosecond intervals since January 1, 1601 (UTC).
    // We need to convert it to microseconds since January 1, 1970 (UTC).

    // Subtract the number of 100-nanosecond intervals between the two dates.
    // The value is 116444736000000000 (obtained from various sources).
    epoch_microseconds = (uli.QuadPart - 116444736000000000ULL) / 10; // Convert to microseconds

    global->stack[global->stack_index].timestamp = epoch_microseconds
#endif

    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u16 i = 0;
        for ( ; i < 100 ; ++i) {
            if (function[i] == '\0') break;
            global->stack[global->stack_index].function[i] = function[i];
        }

        TLMEMSET(&global->stack[global->stack_index].function[i], 0, 100 - i);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    TLMEMSET(global->stack[global->stack_index].arguments, 0, 1024);
    if (arguments != NULL){
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(global->stack[global->stack_index].arguments, 1024, arguments, arg_ptr);
        va_end(arg_ptr);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u32 index = 0;
        const char* character = filename;
        for (u32 i = 0; *character != '\0' ; ++character) {
            if (*character == '/') { index = i + 1; }
            if (++i == U32_MAX) { i = 0; }
        }

        u16 i = 0;
        for ( ; i < 100 ; ++i) {
            if (filename[index + i] == '\0') break;
            global->stack[global->stack_index].filename[i] = filename[index + i];
        }

        TLMEMSET(&global->stack[global->stack_index].filename[i], 0, 100 - i);
    }

    global->stack_index++;
    if (global->stack_index > global->stack_maximum) {
        global->stack_maximum = global->stack_index;
    }

    // TLVERBOSE("STACK PUSH :: %s:%d %s(%s)",
    //     global->stack[global->stack_index].filename, global->stack[global->stack_index].lineno,
    //     global->stack[global->stack_index].function, global->stack[global->stack_index].arguments
    // )
#endif
}

void tl_meta_frame_pop() {
#if ! defined(TELEIOS_BUILD_RELEASE)
    if (global->stack_index == 0) TLWARN("global->stack_index is zero");

    // TLVERBOSE("STACK POP :: %s:%d %s(%s)",
    //     global->stack[global->stack_index].filename, global->stack[global->stack_index].lineno,
    //     global->stack[global->stack_index].function, global->stack[global->stack_index].arguments
    // )

    global->stack_index--;
#endif
}


