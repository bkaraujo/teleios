#ifndef __TL_DEFINES__
#define __TL_DEFINES__


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   define TL_PLATFORM_WINDOWS 1
#   ifndef _WIN64
#       error "64-bit is required on Windows!"
#   endif
#   include <windows.h>
#endif


#if defined(_POSIX_VERSION)
#   define TL_PLATFORM_POSIX 1
#endif


#if defined(__linux__) || defined(__gnu_linux__)
#   define TL_PLATFORM_LINUX 1
#   if defined(__ANDROID__)
#       define TL_PLATFORM_ANDROID 1
#   endif
#endif


#if defined(__unix__)
#   define TL_PLATFORM_UNIX 1
#   include <unistd.h>
#   if defined(_XOPEN_VERSION)
#       if (_XOPEN_VERSION >= 3)
#           define TL_XOPEN_1989
#       endif
#       if (_XOPEN_VERSION >= 4)
#           define TL_XOPEN_1992
#       endif
#       if (_XOPEN_VERSION >= 4) && defined(_XOPEN_UNIX)
#           define TL_XOPEN_1995
#       endif
#       if (_XOPEN_VERSION >= 500)
#           define TL_XOPEN_1998
#       endif
#       if (_XOPEN_VERSION >= 600)
#           define TL_XOPEN_2003
#       endif
#       if (_XOPEN_VERSION >= 700)
#           define TL_XOPEN_2008
#       endif
#   endif
#endif


#if __APPLE__
#   define TL_PLATFORM_APPLE 1
#   include <TargetConditionals.h>
#   if TARGET_IPHONE_SIMULATOR
#       define TL_PLATFORM_IOS 1
#       define TL_PLATFORM_IOS_SIMULATOR 1
#   elif TARGET_OS_IPHONE
#       define TL_PLATFORM_IOS 1
#   elif TARGET_OS_MAC
#   else
#       error "Unknown Apple platform"
#   endif
#endif

#include <stddef.h>	            // Common macro definitions
#include <assert.h>	            // Conditionally compiled macro that compares its argument to zero
#include <ctype.h>	            // Functions to determine the type contained in character data
#include <errno.h>	            // Macros reporting error conditions
#include <float.h>	            // Limits of floating-point types
#include <limits.h>	            // Ranges of integer types
#include <locale.h>	            // Localization utilities
#include <math.h>	            // Common mathematics functions
#include <setjmp.h>	            // Nonlocal jumps
#include <signal.h>	            // Signal handling
#include <stdarg.h>	            // Variable arguments
#include <stdio.h>	            // Input/output
#include <stdlib.h>	            // General utilities: memory management, program utilities, string conversions, random numbers, algorithms
#include <time.h>	            // Time/date utilities
#include <string.h>	            // String handling
#if defined(__STDC_VERSION__)
#    if (__STDC_VERSION__ >= 199409L)
#        if defined(TL_CSTD)
#            undef TL_CSTD
#        endif
#        define TL_CSTD 1995
#        include <iso646.h>       // Alternative operator spellings
#        include <wchar.h>        // Extended multibyte and wide character utilities
#        include <wctype.h>       // Functions to determine the type contained in wide character data
#    endif
#    if (__STDC_VERSION__ >= 199901L)
#        if defined(TL_CSTD)
#            undef TL_CSTD
#        endif
#        define TL_CSTD 1999
#        include <stdint.h>       // Fixed-width integer types
#        include <stdbool.h>      // Macros for boolean type
#        include <inttypes.h>     // Format conversion of integer types
#        include <fenv.h>         // Floating-point environment
#        if !defined(TL_PLATFORM_WINDOWS)
#            include <complex.h>      // Complex number arithmetic (não suportado nativamente no Windows)
#            include <tgmath.h>       // Type-generic math (depende de complex.h)
#        endif
#    endif
#    if (__STDC_VERSION__ >= 201112L)
#        if defined(TL_CSTD)
#            undef TL_CSTD
#        endif
#        define TL_CSTD 2011
#        include <stdnoreturn.h>                    // (deprecated in C23)	noreturn convenience macro
#        include <stdalign.h>                       // (deprecated in C23)	alignas and alignof convenience macros
#        include <stdatomic.h>                      // Atomic operations
#        if !defined(TL_PLATFORM_WINDOWS)
#            include <threads.h>                    // Thread library (não suportado nativamente no MinGW/Windows)
#        endif
#        include <uchar.h>                          // UTF-16 and UTF-32 character utilities
#    endif
#    if (__STDC_VERSION__ >= 201710L)
#        if defined(TL_CSTD)
#            undef TL_CSTD
#        endif
#        define TL_CSTD 2017
#    endif
#    if (__STDC_VERSION__ >= 202311L)
#        if defined(TL_CSTD)
#            undef TL_CSTD
#        endif
#        define TL_CSTD 2023
#        include <stdbit.h>       // Macros to work with the byte and bit representations of types
#        include <stdckdint.h>    // Macros for performing checked integer arithmetic
#    endif
#endif
// ---------------------------------
// Unsigned int types.
// ---------------------------------
/** @brief Unsigned 8-bit integer (0 to 255) */
typedef uint8_t     u8;
/** @brief Unsigned 16-bit integer (0 to 65,535) */
typedef uint16_t    u16;
/** @brief Unsigned 32-bit integer (0 to 4,294,967,295) */
typedef uint32_t    u32;
/** @brief Unsigned 64-bit integer (0 to 18,446,744,073,709,551,615) */
typedef uint64_t    u64;

#define U64_MAX     18446744073709551615
#define U32_MAX     4294967295
#define U16_MAX     65535
#define  U8_MAX     255
#define U64_MIN     0UL
#define U32_MIN     0U
#define U16_MIN     0U
#define  U8_MIN     0U
// ---------------------------------
// Signed int types.
// ---------------------------------
/** @brief Signed 8-bit integer (-128 to 127) */
typedef int8_t      i8;
/** @brief Signed 16-bit integer (-32,768 to 32,767) */
typedef int16_t     i16;
/** @brief Signed 32-bit integer (-2,147,483,648 to 2,147,483,647) */
typedef int32_t     i32;
/** @brief Signed 64-bit integer (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807) */
typedef int64_t     i64;

#define  I8_MAX     127
#define I16_MAX     32767
#define I32_MAX     2147483647
#define I64_MAX     9223372036854775807

#define  I8_MIN     (-127 - 1)
#define I16_MIN     (-32767 - 1)
#define I32_MIN     (-2147483647 - 1)
#define I64_MIN     (-9223372036854775807 - 1)
// ---------------------------------
// Floating point types
// ---------------------------------
/** @brief 32-bit floating point (IEEE 754 single precision) */
typedef float       f32;
/** @brief 64-bit floating point (IEEE 754 double precision) */
typedef double      f64;
// ---------------------------------
// Boolean types
// ---------------------------------
/** @brief Boolean type (true/false) */
typedef _Bool       b8;
// ---------------------------------
// Static assertions.
// ---------------------------------
#if defined(TL_CSTD) && (TL_CSTD >= 2011)
#   if defined(__clang__) || defined(__GNUC__)
#       define STATIC_ASSERT _Static_assert
#   else
#       define STATIC_ASSERT static_assert
#   endif
STATIC_ASSERT(sizeof(u8) == 1, "Expected  u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected  i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");
#endif
// ---------------------------------
// Compiler specifics
// ---------------------------------
#if defined(__clang__) || defined(__GNUC__)
/** @brief Force function to be inlined (GCC/Clang) */
#   define TL_INLINE __attribute__((always_inline)) inline
/** @brief Prevent function from being inlined (GCC/Clang) */
#   define TL_NOINLINE __attribute__((noinline))
/** @brief Mark function as deprecated with custom message (GCC/Clang) */
#   define TL_DEPRECATED(message) __attribute__((deprecated(message)))
/** @brief Branch prediction hint: condition is likely true */
#   define TL_LIKELY(x)   __builtin_expect(!!(x), 1)
/** @brief Branch prediction hint: condition is unlikely true */
#   define TL_UNLIKELY(x) __builtin_expect(!!(x), 0)
/** @brief Thread-local storage specifier (GCC/Clang) */
#   define TL_THREADLOCAL _Thread_local
#   if defined(TL_EXPORT)
/** @brief Export symbol for dynamic library (GCC/Clang) */
#       define TL_API __attribute__((visibility("default")))
#   endif
#elif defined(_MSC_VER)
/** @brief Force function to be inlined (MSVC) */
#   define TL_INLINE __forceinline
/** @brief Prevent function from being inlined (MSVC) */
#   define TL_NOINLINE __declspec(noinline)
/** @brief Mark function as deprecated with custom message (MSVC) */
#   define TL_DEPRECATED(message) __declspec(deprecated(message))
/** @brief Thread-local storage specifier (MSVC) */
#   define TL_THREADLOCAL __declspec(thread)
#   if defined(TL_EXPORT)
/** @brief Export symbol for DLL (MSVC) */
#       define TL_API __declspec(dllexport)
#   elif defined(TL_IMPORT)
/** @brief Import symbol from DLL (MSVC) */
#       define TL_API __declspec(dllimport)
#   endif
#endif

#if ! defined(TL_LIKELY)
#   define TL_LIKELY(x)   (x)
#endif

#if ! defined(TL_UNLIKELY)
#   define TL_UNLIKELY(x) (x)
#endif

#if ! defined(TL_API)
#   define TL_API
#endif

#if ! defined(TL_INLINE)
#   define TL_INLINE static inline
#endif

#if ! defined(TL_NOINLINE)
#   define TL_NOINLINE
#endif

#if ! defined(TL_DEPRECATED)
#   define TL_DEPRECATED(message)
#endif
// ---------------------------------
// Helper Functions
// ---------------------------------
/** @brief Convert amount to bytes using binary kibi (1024 bytes) */
#define TL_KIBI_BYTES(amount) ((amount) * 1024ULL)
/** @brief Convert amount to bytes using binary mebi (1024^2 bytes) */
#define TL_MEBI_BYTES(amount) ((amount) * 1024ULL * 1024ULL)
/** @brief Convert amount to bytes using binary gibi (1024^3 bytes) */
#define TL_GIBI_BYTES(amount) ((amount) * 1024ULL * 1024ULL * 1024ULL)

/** @brief Convert amount to bytes using decimal kilo (1000 bytes) */
#define TL_KILO_BYTES(amount) ((amount) * 1000ULL)
/** @brief Convert amount to bytes using decimal mega (1000^2 bytes) */
#define TL_MEGA_BYTES(amount) ((amount) * 1000ULL * 1000ULL)
/** @brief Convert amount to bytes using decimal giga (1000^3 bytes) */
#define TL_GIGA_BYTES(amount) ((amount) * 1000ULL * 1000ULL * 1000ULL)

#include <cglm/types-struct.h>

/**
 * @brief Log severity levels
 *
 * Ordered from least to most severe. The logger only outputs messages
 * at or above the configured level. Lower enum values = lower severity.
 *
 * VERBOSE, TRACE, and DEBUG levels are automatically compiled out in
 * Release builds for zero runtime overhead.
 *
 * @see tl_logger_loglevel
 */
typedef enum {
    TL_LOG_LEVEL_VERBOSE = 0,   ///< Very detailed diagnostic information (Debug-only)
    TL_LOG_LEVEL_TRACE   = 1,   ///< Function entry/exit tracing (Debug-only)
    TL_LOG_LEVEL_DEBUG   = 2,   ///< General debugging messages (Debug-only)
    TL_LOG_LEVEL_INFO    = 3,   ///< Informational messages about normal operation
    TL_LOG_LEVEL_WARN    = 4,   ///< Warnings about potentially problematic conditions
    TL_LOG_LEVEL_ERROR   = 5,   ///< Error conditions that indicate failures
    TL_LOG_LEVEL_FATAL   = 6    ///< Fatal errors that terminate execution
} TLLogLevel;

typedef enum {
    TL_DISPLAY_RESOLUTION_SD  = 480,
    TL_DISPLAY_RESOLUTION_HD  = 720,
    TL_DISPLAY_RESOLUTION_FHD = 1080,
    TL_DISPLAY_RESOLUTION_QHD = 1440,
    TL_DISPLAY_RESOLUTION_UHD = 2160
} TLDisplayResolution;

/**
 * @brief Opaque allocator handle
 *
 * Represents a memory allocator instance. The actual structure is defined
 * in the implementation files (memory_linear.inl, memory_dynamic.inl).
 */
typedef struct TLAllocator TLAllocator;

/**
 * @brief Current date and time representation
 *
 * Holds a breakdown of the current date and time into individual components.
 * This structure is returned by tl_time_clock().
 *
 * @note Year is stored as a full 4-digit year (e.g., 2025)
 * @note Month is 1-12 (not 0-11)
 * @note Day is 1-31
 * @note Hour is 0-23
 * @note Minute is 0-59
 * @note Second is 0-59
 * @note Millis is 0-999
 *
 * @see tl_time_clock
 */
typedef struct {
  u16 millis;                 ///< Milliseconds (0-999)
  u16 year;                   ///< Full year (e.g., 2025)
  u8 month;                   ///< Month (1-12)
  u8 day;                     ///< Day of month (1-31)
  u8 hour;                    ///< Hour (0-23)
  u8 minute;                  ///< Minute (0-59)
  u8 second;                  ///< Second (0-59)
} TLDateTime;

typedef struct TLArray TLArray;

/**
 * @brief Opaque queue data structure handle
 *
 * Represents a circular queue instance. The actual structure definition
 * is in the implementation file (container.c).
 */
typedef struct TLQueue TLQueue;

typedef struct TLListNode TLListNode;

typedef struct TLList TLList;

typedef struct TLMapEntry TLMapEntry;

typedef struct TLMap TLMap;

/**
 * @brief Iterator structure for snapshot-based iteration
 *
 * Iterator that creates a snapshot of a list at creation time.
 * Optimized for hot loops with contiguous memory layout and lock-free iteration.
 */
typedef struct TLIterator TLIterator;

/**
 * @brief Opaque object pool handle
 *
 * Represents a pool of pre-allocated objects for efficient reuse.
 * The actual structure definition is in the implementation file (container.c).
 */
typedef struct TLObjectPool TLObjectPool;

typedef struct TLThread TLThread;

typedef struct TLMutex TLMutex;

typedef struct TLCondition TLCondition;

/**
 * @brief String object structure
 *
 * Encapsulates a null-terminated C string with cached length and allocator
 * reference. Strings are immutable - all transformation operations return new
 * instances.
 */
typedef struct TLString TLString;

typedef struct  TLStackTrace TLStackTrace;

extern TLAllocator* g_allocator;
#endif
