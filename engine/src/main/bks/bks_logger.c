#include "bks/bks.h"
#include <stdio.h>
#include <pthread.h>

#if defined(BKS_PLATFORM_WINDOWS)
#   define ANSI_COLOR_FATAL   ""
#   define ANSI_COLOR_ERROR   ""
#   define ANSI_COLOR_WARN    ""
#   define ANSI_COLOR_INFO    ""
#   define ANSI_COLOR_DEBUG   ""
#   define ANSI_COLOR_TRACE   ""
#   define ANSI_COLOR_VERBOSE ""
#endif

#if defined(BKS_PLATFORM_LINUX)
#   define ANSI_COLOR_FATAL   "\033[1;31m"
#   define ANSI_COLOR_ERROR   "\033[1;31m"
#   define ANSI_COLOR_WARN    "\033[1;33m"
#   define ANSI_COLOR_INFO    "\033[1;32m"
#   define ANSI_COLOR_DEBUG   "\033[1;34m"
#   define ANSI_COLOR_TRACE   "\033[1;36m"
#   define ANSI_COLOR_VERBOSE "\033[1;37m"
#endif

static const char *strings[] = {"VERBOSE ", "TRACE  ", "DEBUG  ", "INFO   ", "WARN   ", "ERROR  ", "FATAL  "};
static const char *colors[] = { ANSI_COLOR_VERBOSE , ANSI_COLOR_TRACE, ANSI_COLOR_DEBUG, ANSI_COLOR_INFO, ANSI_COLOR_WARN, ANSI_COLOR_ERROR, ANSI_COLOR_FATAL };
static BKSLogLevel m_level = BKS_LOG_LEVEL_INFO;

void bks_logger_loglevel(const BKSLogLevel desired){
    m_level = desired;
}

void bks_logger_write(const BKSLogLevel level, const char *filename, const u32 lineno, const char *message, ...) {
    if (level < m_level) { return; }

    char output[2048] = { 0 };
    __builtin_va_list arg_ptr;
    __builtin_va_start(arg_ptr, message);
    __builtin_vsnprintf(output, 2048, message, arg_ptr);
    __builtin_va_end(arg_ptr);

    u16 index = 0;
    const char* character = filename;
    for (u16 i = 0; *character != '\0' ; ++character) {
        if (*character == '/') { index = i + 1; }
        if (i++ == U32_MAX) { i = 0; }
    }

    BKSClock clock; bks_time_clock(&clock);
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
// #if ! defined(BKS_BUILD_RELEASE)
//     // Print the stack trace
//     if (level == BKS_LOG_LEVEL_FATAL) {
//         const char *format = "%66s at %20s:%04d %s(%s)\n\033[1;30m";
//
//         fprintf(stdout, "\n");
//         for (u8 i = stack_index ; i > 0 ; --i) {
//             fprintf(stdout, format, colors[level], stack[i].filename, stack[i].lineno, stack[i].function, stack[i].arguments);
//         }
//
//         fprintf(stdout, format, colors[level], stack[0].filename, stack[0].lineno, stack[0].function, stack[0].arguments);
//     }
// #endif
}