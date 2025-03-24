#include <stdio.h>
#include <stdarg.h>
#include "teleios/core.h"

#if ! defined(TELEIOS_BUILD_RELEASE)
#   include "teleios/globals.h"
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
#   include <time.h>
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

    u32 index = 0;
    const char* character = filename;
    for (u16 i = 0; *character != '\0' ; ++character) {
        if (*character == '/') { index = i + 1; }
        if (++i == U32_MAX) { i = 0; }
    }

#ifdef TLPLATFORM_LINUX
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) return;

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

    fprintf(stdout, "%s%d-%02d-%02d %02d:%02d:%02d,%06ld %20s:%04d %s %s\n\033[1;30m",
        colors[level],
        localtime.tm_year + 1900, localtime.tm_mon + 1, localtime.tm_mday,
        localtime.tm_hour, localtime.tm_min, localtime.tm_sec, now.tv_nsec / 1000,
        filename + index, lineno,
        strings[level],
        output
    );
#endif

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