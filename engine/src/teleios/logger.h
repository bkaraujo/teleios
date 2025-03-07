#ifndef __TELEIOS_LOGGER__
#define __TELEIOS_LOGGER__

#include "teleios/defines.h"
#include <stdlib.h>

typedef enum {
    TL_LOG_LEVEL_FATAL   = 0,
    TL_LOG_LEVEL_ERROR   = 1,
    TL_LOG_LEVEL_WARN    = 2,
    TL_LOG_LEVEL_INFO    = 3,
    TL_LOG_LEVEL_DEBUG   = 4,
    TL_LOG_LEVEL_TRACE   = 5,
    TL_LOG_LEVEL_VERBOSE = 6
} TLLogLevel;

void tl_logger_write(TLLogLevel level, const char* filename, u32 lineno, const char* message, ...);

#define TLFATAL(m, ...) { tl_logger_write(TL_LOG_LEVEL_FATAL, __FILE__, __LINE__, m, ##__VA_ARGS__); exit(99); }
#define TLERROR(m, ...) { tl_logger_write(TL_LOG_LEVEL_ERROR, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define TLWARN(m, ...) { tl_logger_write(TL_LOG_LEVEL_WARN, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define TLINFO(m, ...) { tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define TLDEBUG(m, ...) { tl_logger_write(TL_LOG_LEVEL_DEBUG, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define TLTRACE(m, ...) { tl_logger_write(TL_LOG_LEVEL_TRACE, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define TLVERBOSE(m, ...) { tl_logger_write(TL_LOG_LEVEL_VERBOSE, __FILE__, __LINE__, m, ##__VA_ARGS__); }

#endif // __TELEIOS_LOGGER__