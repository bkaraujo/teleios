#ifndef __LIBK_LOGGER__
#define __LIBK_LOGGER__

#include "libk/defines.h"

typedef enum {
    K_LOG_LEVEL_VERBOSE = 0,
    K_LOG_LEVEL_TRACE   = 1,
    K_LOG_LEVEL_DEBUG   = 2,
    K_LOG_LEVEL_INFO    = 3,
    K_LOG_LEVEL_WARN    = 4,
    K_LOG_LEVEL_ERROR   = 5,
    K_LOG_LEVEL_FATAL   = 6
} KLogLevel;

void k_logger_loglevel(KLogLevel desired);
void k_logger_write(KLogLevel level, const char *filename, u32 lineno, const char *message, ...);

#if ! defined(K_BUILD_RELEASE)
#   define KVERBOSE(m, ...) { k_logger_write(K_LOG_LEVEL_VERBOSE, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#   define   KTRACE(m, ...) { k_logger_write(K_LOG_LEVEL_TRACE  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#   define   KDEBUG(m, ...) { k_logger_write(K_LOG_LEVEL_DEBUG  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#else
#   define KVERBOSE(m, ...)
#   define   KTRACE(m, ...)
#   define   KDEBUG(m, ...)
#endif

#define    KINFO(m, ...) { k_logger_write(K_LOG_LEVEL_INFO   , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define    KWARN(m, ...) { k_logger_write(K_LOG_LEVEL_WARN   , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define   KERROR(m, ...) { k_logger_write(K_LOG_LEVEL_ERROR  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#define   KFATAL(m, ...) { k_logger_write(K_LOG_LEVEL_FATAL  , __FILE__, __LINE__, m, ##__VA_ARGS__); exit(99); }

#endif