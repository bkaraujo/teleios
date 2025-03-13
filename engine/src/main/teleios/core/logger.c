/*
 * Logging.c
 *
 * Performs logging into the platform STDOUT. It's expected to be possible to call the logger macros from within 
 * any other compilation unit, and for that the logger must have no dependencies.
 */
#include "teleios/core/platform_detector.h"
#include "teleios/core/logger.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef TLPLATFORM_LINUX
#   define __USE_POSIX 1
#   define __USE_POSIX199309 1
#   include <time.h>
#endif

static void tl_logger_forward(TLLogLevel level, const char *filename, u32 lineno, const char *message) {
    static const char *strings[] = {"FATAL   ", "ERROR  ", "WARN   ", "INFO   ", "DEBUG  ", "TRACE  ", "VERBOSE"};

    void *output = TLALLOCA(4096);
    TLMEMSET(output, 0, sizeof(output));
    
    TLClock clock = { 0 }; 
    #ifdef TLPLATFORM_LINUX
        struct timespec now = { 0 };
        if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) return;

        struct tm localtime = { 0 };
        if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

        clock.year = localtime.tm_year + 1900;
        clock.month = localtime.tm_mon + 1;
        clock.day = localtime.tm_mday;
        clock.hour = localtime.tm_hour;
        clock.minute = localtime.tm_min;
        clock.second = localtime.tm_sec;
        clock.millis = now.tv_nsec / 1000;
    #endif

    sprintf(output, "%d-%02d-%02d %02d:%02d:%02d,%05d %20s:%03d %s %s\n", 
        clock.year, clock.month, clock.day,
        clock.hour, clock.minute, clock.second, clock.millis,
        filename, lineno,
        strings[level], 
        ((char*) message)
    );

    #ifdef TLPLATFORM_LINUX
        static const char *colors[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;22", "1;30"};
        fprintf(stdout, "\033[%sm%s\033[0m", colors[level], output);
    #endif
}

void tl_logger_write(TLLogLevel level, const char *filename, u32 lineno, const char *message, ...) {
    void *output = TLALLOCA(4061);
    TLMEMSET(output, 0, sizeof(output));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(((char*) output), 4096, message, arg_ptr);
    va_end(arg_ptr);

    tl_logger_forward(level, __builtin_strrchr(filename, '/') + 1, lineno, output);
}
