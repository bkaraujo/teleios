#include "teleios/teleios.h"

#ifndef TELEIOS_LOG_LENGTH
#   define TELEIOS_LOG_LENGTH 1024
#endif

#if defined(TELEIOS_BUILD_DEBUG)
static TLLogLevel m_level = TL_LOG_LEVEL_TRACE;
#else
static TLLogLevel m_level = TL_LOG_LEVEL_INFO;
#endif
TLLogLevel tl_logger_get_level(void) {
    return m_level;
}
void tl_logger_set_level(const TLLogLevel desired){
    m_level = desired;
}

static const char *strings[] = {"VERBOSE", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
static const char *colors[] = { "\033[1;37m" , "\033[1;36m", "\033[1;34m", "\033[1;32m", "\033[1;33m", "\033[1;31m", "\033[1;31m" };

void tl_logger_write(const TLLogLevel level, const char *filename, const u32 lineno, const char *message, ...) {
    if (level < m_level) { return; }

    static TL_THREADLOCAL char output[TELEIOS_LOG_LENGTH];
    va_list arg_ptr; va_start(arg_ptr, message);
    vsnprintf(output, TELEIOS_LOG_LENGTH, message, arg_ptr);
    va_end(arg_ptr);

    const char* slash = strrchr(filename, '/');         // strrchr Otimizado (SIMD)
    const char* backslash = strrchr(filename, '\\');    // strrchr Otimizado (SIMD)
    const char* basename = (slash > backslash) ? slash + 1 : (backslash ? backslash + 1 : filename);

    static TL_THREADLOCAL TLDateTime clock;
    tl_time_clock(&clock);
    
    static TL_THREADLOCAL char buffer[TELEIOS_LOG_LENGTH];
    const int len = snprintf(buffer, TELEIOS_LOG_LENGTH, "%s%d-%02d-%02d %02d:%02d:%02d,%06u %6llu %20s:%04d %-7s %s\n\033[1;30m",
        colors[level],
        clock.year, clock.month, clock.day,
        clock.hour, clock.minute, clock.second, clock.millis,
        tl_thread_current_id(),
        basename, 
        lineno, 
        strings[level], 
        output
    );
    
    if (len > 0) fwrite(buffer, 1, len, stdout);
}