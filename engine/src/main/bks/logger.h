#ifndef __BK_STANDARD_LOGGER__
#define __BK_STANDARD_LOGGER__

#include "bks/defines.h"
#include <stdlib.h>

typedef enum {
    BKS_LOG_LEVEL_VERBOSE = 0,
    BKS_LOG_LEVEL_TRACE   = 1,
    BKS_LOG_LEVEL_DEBUG   = 2,
    BKS_LOG_LEVEL_INFO    = 3,
    BKS_LOG_LEVEL_WARN    = 4,
    BKS_LOG_LEVEL_ERROR   = 5,
    BKS_LOG_LEVEL_FATAL   = 6
} BKSLogLevel;

void bks_logger_loglevel(BKSLogLevel desired);
void bks_logger_write(BKSLogLevel level, const char *filename, u32 lineno, const char *message, ...);

#if ! defined(BKS_BUILD_RELEASE)
#   define BKSVERBOSE(m, ...) { bks_logger_write(BKS_LOG_LEVEL_VERBOSE, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#   define   BKSTRACE(m, ...) { bks_logger_write(BKS_LOG_LEVEL_TRACE  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#   define   BKSDEBUG(m, ...) { bks_logger_write(BKS_LOG_LEVEL_DEBUG  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#else
#   define BKSVERBOSE(m, ...)
#   define   BKSTRACE(m, ...)
#   define   BKSDEBUG(m, ...)
#endif

#define    BKSINFO(m, ...) { bks_logger_write(BKS_LOG_LEVEL_INFO   , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define    BKSWARN(m, ...) { bks_logger_write(BKS_LOG_LEVEL_WARN   , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define   BKSERROR(m, ...) { bks_logger_write(BKS_LOG_LEVEL_ERROR  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define   BKSFATAL(m, ...) { bks_logger_write(BKS_LOG_LEVEL_FATAL  , __FILE__, __LINE__, m, ##__VA_ARGS__); exit(99); }

#endif