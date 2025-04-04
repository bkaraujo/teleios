#include "core.h"
// ########################################################
//                    TIME FUNCTIONS
// ########################################################
#include <time.h>
#include <pthread.h>
#if defined(TLPLATFORM_LINUX)
#include <sys/time.h>
#endif

void tl_time_clock(TLClock* clock) {
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) return;

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

    clock->year = localtime.tm_year + 1900;
    clock->month = localtime.tm_mon + 1;
    clock->day = localtime.tm_mday;
    clock->hour = localtime.tm_hour;
    clock->minute = localtime.tm_min;
    clock->second = localtime.tm_sec;
    clock->millis = now.tv_nsec / 1000;
}

u64 tl_time_epoch_millis(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME_COARSE, &now);
    return (uint64_t) now.tv_sec * 1000000 + now.tv_nsec;
}

u64 tl_time_epoch_micros(void) {
    u64 micros = 0;
#if defined(TLPLATFORM_LINUX)
    struct timeval now = { 0 };
    gettimeofday(&now, NULL);
    micros = (uint64_t) now.tv_sec * 1000000 + now.tv_usec;
#elif defined(TLPLATFORM_WINDOWS)
    FILETIME ft; GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // FILETIME is in 100-nanosecond intervals since January 1, 1601 (UTC).
    // We need to convert it to microseconds since January 1, 1970 (UTC).

    // Subtract the number of 100-nanosecond intervals between the two dates.
    // The value is 116444736000000000 (obtained from various sources).
    micros = (uli.QuadPart - 116444736000000000ULL) / 10; // Convert to microseconds
#endif
    return micros;
}
// #####################################################################################################################
//
//                                                     LOGGER
//
// #####################################################################################################################
#if ! defined(TELEIOS_BUILD_RELEASE)
#include "teleios/globals.h"
#endif

#if defined(TLPLATFORM_WINDOWS)
#   define ANSI_COLOR_FATAL   ""
#   define ANSI_COLOR_ERROR   ""
#   define ANSI_COLOR_WARN    ""
#   define ANSI_COLOR_INFO    ""
#   define ANSI_COLOR_DEBUG   ""
#   define ANSI_COLOR_TRACE   ""
#   define ANSI_COLOR_VERBOSE ""
#endif

#if defined(TLPLATFORM_LINUX)
#   define ANSI_COLOR_FATAL   "\033[1;31m"
#   define ANSI_COLOR_ERROR   "\033[1;31m"
#   define ANSI_COLOR_WARN    "\033[1;33m"
#   define ANSI_COLOR_INFO    "\033[1;32m"
#   define ANSI_COLOR_DEBUG   "\033[1;34m"
#   define ANSI_COLOR_TRACE   "\033[1;36m"
#   define ANSI_COLOR_VERBOSE "\033[1;37m"
#endif

#if defined(TELEIOS_BUILD_RELEASE)
static TLLogLevel m_level = TL_LOG_LEVEL_INFO;
#else
static TLLogLevel m_level = TL_LOG_LEVEL_VERBOSE;
#endif
static const char *strings[] = {"VERBOSE ", "TRACE  ", "DEBUG  ", "INFO   ", "WARN   ", "ERROR  ", "FATAL  "};
static const char *colors[] = { ANSI_COLOR_VERBOSE , ANSI_COLOR_TRACE, ANSI_COLOR_DEBUG, ANSI_COLOR_INFO, ANSI_COLOR_WARN, ANSI_COLOR_ERROR, ANSI_COLOR_FATAL };

void tl_logger_loglevel(const TLLogLevel desired){
    m_level = desired;
}

void tl_logger_write(const TLLogLevel level, const char *filename, const u32 lineno, const char *message, ...) {
    if (level < m_level) { return; }

    char output[2048] = { 0 };
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(output, 2048, message, arg_ptr);
    va_end(arg_ptr);

    u16 index = 0;
    const char* character = filename;
    for (u16 i = 0; *character != '\0' ; ++character) {
        if (*character == '/') { index = i + 1; }
        if (i++ == U32_MAX) { i = 0; }
    }

    TLClock clock; tl_time_clock(&clock);
    fprintf(stdout, "%s%d-%02d-%02d %02d:%02d:%02d,%06u %lu %20s:%04d %s %s\n\033[1;30m",
        colors[level],
        clock.year, clock.month, clock.day,
        clock.hour, clock.minute, clock.second, clock.millis,
        pthread_self(),
        filename + index, lineno,
        strings[level],
        output
    );

    fflush(stdout);
#if ! defined(TELEIOS_BUILD_RELEASE)
    // Print the stack trace
    if (level == TL_LOG_LEVEL_FATAL) {
        const char *format = "%66s at %20s:%04d %s(%s)\n\033[1;30m";

        fprintf(stdout, "\n");
        for (u8 i = global->stack_index ; i > 0 ; --i) {
            fprintf(stdout, format, colors[level], global->stack[i].filename, global->stack[i].lineno, global->stack[i].function, global->stack[i].arguments);
        }

        fprintf(stdout, format, colors[level], global->stack[0].filename, global->stack[0].lineno, global->stack[0].function, global->stack[0].arguments);
    }
#endif
}
// #####################################################################################################################
//
//                                                     META
//
// #####################################################################################################################
#if ! defined(TELEIOS_BUILD_RELEASE)
#   include <stdio.h>
#endif

void tl_trace_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
#if ! defined(TELEIOS_BUILD_RELEASE)

    global->stack_index++;
    if (global->stack_index >= TL_STACK_SIZE_MAXIMUM) {
        TLFATAL("global->stack_index exceeded")
    }

    if (global->stack_index > global->stack_maximum) {
        global->stack_maximum = global->stack_index;
    }

    global->stack[global->stack_index].lineno = lineno;
    global->stack[global->stack_index].timestamp = tl_time_epoch_micros();
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u16 i = 0;
        for ( ; i < TL_STACK_STRING_SIZE ; ++i) {
            if (function[i] == '\0') break;
            global->stack[global->stack_index].function[i] = function[i];
        }

        tl_platform_memory_set(&global->stack[global->stack_index].function[i], 0, TL_STACK_STRING_SIZE - i);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    tl_platform_memory_set(global->stack[global->stack_index].arguments, 0, TL_STACK_ARGUMENTS_SIZE);
    if (arguments != NULL){
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(global->stack[global->stack_index].arguments, TL_STACK_ARGUMENTS_SIZE, arguments, arg_ptr);
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
        for ( ; i < TL_STACK_STRING_SIZE ; ++i) {
            if (filename[index + i] == '\0') break;
            global->stack[global->stack_index].filename[i] = filename[index + i];
        }

        tl_platform_memory_set(&global->stack[global->stack_index].filename[i], 0, TL_STACK_STRING_SIZE - i);
    }
#endif
}

void tl_trace_pop() {
#if ! defined(TELEIOS_BUILD_RELEASE)
    tl_platform_memory_set(&global->stack[global->stack_index], 0 , sizeof(TLStackFrame));
    global->stack_index--;
#endif
}

typedef struct {
    const char* name;
    u64 timestamp;
    u64 ticks;
} TLProfile;

static TLProfile profile[U8_MAX];

void tl_profiler_begin(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    u32 index = 0;
    for (u32 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) {
            index = i;
            break;
        }
    }

    profile[index].name = name;
    profile[index].timestamp = tl_time_epoch_micros();
#endif
}

static u8 tl_profiler_index(const char *name) {
    for (u8 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) continue;

        b8 found = true;
        for (u64 j = 0; j < U64_MAX; ++j) {
            if (profile[i].name[j] != name[j]) {
                found = false;
                break;
            }

            if (name[j] == '\0') break;
        }

        if (found) return i;
    }

    TLFATAL("Profile [%s] not found", name)
}

u64 tl_profiler_time(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    return tl_time_epoch_micros() - profile[index].timestamp;
#endif
}

void tl_profiler_tick(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    profile[index].ticks++;
#endif
}

u64 tl_profiler_ticks(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    return profile[index].ticks;
#else
    return U64_MAX;
#endif
}

void tl_profiler_end(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    tl_platform_memory_set(&profile[index], 0, sizeof(TLProfile));
#endif
}
