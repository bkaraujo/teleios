#include "teleios/logger.h"
#include "teleios/chrono.h"
#include "teleios/filesystem.h"

static TLLogLevel m_level = TL_LOG_LEVEL_VERBOSE;
void tl_logger_loglevel(const TLLogLevel desired){
    m_level = desired;
}

static const char *strings[] = {"VERBOSE", "TRACE  ", "DEBUG  ", "INFO   ", "WARN   ", "ERROR  ", "FATAL  "};
static const char *colors[] = { "\033[1;37m" , "\033[1;36m", "\033[1;34m", "\033[1;32m", "\033[1;33m", "\033[1;31m", "\033[1;31m" };

void tl_logger_write(const TLLogLevel level, const char *filename, const u32 lineno, const char *message, ...) {
    if (level < m_level) { return; }

    char output[2048] = { 0 };
    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(output, 2048, message, arg_ptr);
    va_end(arg_ptr);

    u16 index = 0;
    const char* character = filename;
    for (u16 i = 0; *character != '\0' ; ++character) {
        if (*character == tl_filesystem_path_separator() ) { index = i + 1; }
        if (i++ == U16_MAX) { i = 0; }
    }

    TLDateTime clock; tl_time_clock(&clock);
    fprintf(stdout, "%s%d-%02d-%02d %02d:%02d:%02d,%06u %s:%04d %s %s\n\033[1;30m",
        colors[level],
        clock.year, clock.month, clock.day,
        clock.hour, clock.minute, clock.second, clock.millis,
        filename + index, lineno,
        strings[level],
        output
    );

    fflush(stdout);
}