#ifndef __TELEIOS_CORE_LOGGER__
#define __TELEIOS_CORE_LOGGER__

#include "teleios/defines.h"
#include <stdlib.h>

typedef enum {
    TL_LOG_LEVEL_VERBOSE = 0,
    TL_LOG_LEVEL_TRACE   = 1,
    TL_LOG_LEVEL_DEBUG   = 2,
    TL_LOG_LEVEL_INFO    = 3,
    TL_LOG_LEVEL_WARN    = 4,
    TL_LOG_LEVEL_ERROR   = 5,
    TL_LOG_LEVEL_FATAL   = 6
} TLLogLevel;

void tl_logger_loglevel(const TLLogLevel desired);

void tl_logger_write(const TLLogLevel level, const char *filename, const u32 lineno, const char *message, ...);

#define TLVERBOSE(m, ...) { tl_logger_write(TL_LOG_LEVEL_VERBOSE, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define   TLTRACE(m, ...) { tl_logger_write(TL_LOG_LEVEL_TRACE  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define   TLDEBUG(m, ...) { tl_logger_write(TL_LOG_LEVEL_DEBUG  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define    TLINFO(m, ...) { tl_logger_write(TL_LOG_LEVEL_INFO   , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define    TLWARN(m, ...) { tl_logger_write(TL_LOG_LEVEL_WARN   , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define   TLERROR(m, ...) { tl_logger_write(TL_LOG_LEVEL_ERROR  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define   TLFATAL(m, ...) { tl_logger_write(TL_LOG_LEVEL_FATAL  , __FILE__, __LINE__, m, ##__VA_ARGS__); exit(99); }

#endif // __TELEIOS_CORE_LOGGER__