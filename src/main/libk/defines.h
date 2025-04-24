#ifndef __LIBK_DEFINES__
#define __LIBK_DEFINES__

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   define K_PLATFORM_WINDOWS 1
#   ifndef _WIN64
#       error "64-bit is required on Windows!"
#   endif
#   include <windows.h>
#   if defined(K_PATH_SEPARATOR)
#       undef K_PATH_SEPARATOR
#   endif
#   define K_PATH_SEPARATOR '\\'
#endif

#if defined(_POSIX_VERSION)
#   define K_PLATFORM_POSIX 1
#   if defined(K_PATH_SEPARATOR)
#       undef K_PATH_SEPARATOR
#   endif
#   define K_PATH_SEPARATOR '/'
#endif

#if defined(__linux__) || defined(__gnu_linux__)
#   define K_PLATFORM_LINUX 1
#   if defined(K_PATH_SEPARATOR)
#       undef K_PATH_SEPARATOR
#   endif
#   define K_PATH_SEPARATOR '/'
#   if defined(__ANDROID__)
#       define K_PLATFORM_ANDROID 1
#   endif
#endif

#if defined(__unix__)
#   define K_PLATFORM_UNIX 1
#   if defined(K_PATH_SEPARATOR)
#       undef K_PATH_SEPARATOR
#   endif
#   define K_PATH_SEPARATOR '/'
#   include <unistd.h>
#   if defined(_XOPEN_VERSION)
#       if (_XOPEN_VERSION >= 3)
#           define K_XOPEN_1989
#       endif
#       if (_XOPEN_VERSION >= 4)
#           define K_XOPEN_1992
#       endif
#       if (_XOPEN_VERSION >= 4) && defined(_XOPEN_UNIX)
#           define K_XOPEN_1995
#       endif
#       if (_XOPEN_VERSION >= 500)
#           define K_XOPEN_1998
#       endif
#       if (_XOPEN_VERSION >= 600)
#           define K_XOPEN_2003
#       endif
#       if (_XOPEN_VERSION >= 700)
#           define K_XOPEN_2008
#       endif
#   endif
#endif

#if __APPLE__
#   define K_PLATFORM_APPLE 1
#   if defined(K_PATH_SEPARATOR)
#       undef K_PATH_SEPARATOR
#   endif
#   define K_PATH_SEPARATOR '/'
#   include <TargetConditionals.h>
#   if TARGET_IPHONE_SIMULATOR
#       define K_PLATFORM_IOS 1
#       define K_PLATFORM_IOS_SIMULATOR 1
#   elif TARGET_OS_IPHONE
#       define K_PLATFORM_IOS 1
#   elif TARGET_OS_MAC
#   else
#       error "Unknown Apple platform"
#   endif
#endif


#define K_CSTD 1989
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
#include <malloc.h>             // Stack allocation

#if defined(__STDC_VERSION__)
#    if (__STDC_VERSION__ >= 199409L)
#        if defined(K_CSTD)
#            undef K_CSTD
#        endif
#        define K_CSTD 1995
#        include <iso646.h>       // Alternative operator spellings
#        include <wchar.h>        // Extended multibyte and wide character utilities
#        include <wctype.h>       // Functions to determine the type contained in wide character data
#    endif
#    if (__STDC_VERSION__ >= 199901L)
#        if defined(K_CSTD)
#            undef K_CSTD
#        endif
#        define K_CSTD 1999
#        include <complex.h>      // Complex number arithmetic
#        include <fenv.h>         // Floating-point environment
#        include <inttypes.h>     // Format conversion of integer types
#        include <tgmath.h>       // Type-generic math (macros wrapping math.h and complex.h)
#        include <stdint.h>       // Fixed-width integer types
#        include <stdbool.h>      // Macros for boolean type
#        if defined(K_PLATFORM_LINUX)
#            include <pthread.h>  // Thread library
#        endif
#    endif
#    if (__STDC_VERSION__ >= 201112L)
#       if defined(K_CSTD)
#           undef K_CSTD
#       endif
#       define K_CSTD 2011
#       include <stdnoreturn.h>  // (deprecated in C23)	noreturn convenience macro
#       include <stdalign.h>     // (deprecated in C23)	alignas and alignof convenience macros
#       if defined(K_PLATFORM_LINUX)
#        include <stdatomic.h>    // Atomic operations
#       endif
#       include <uchar.h>        // UTF-16 and UTF-32 character utilities
#    endif
#    if (__STDC_VERSION__ >= 201710L)
#        if defined(K_CSTD)
#            undef K_CSTD
#        endif
#        define K_CSTD 2017
#    endif
#    if (__STDC_VERSION__ >= 202311L)
#        if defined(K_CSTD)
#           undef K_CSTD
#        endif
#        define K_CSTD 2023
#        include <stdbit.h>       // Macros to work with the byte and bit representations of types
#        include <stdckdint.h>    // Macros for performing checked integer arithmetic
#    endif
#endif

// ---------------------------------
// Unsigned int types.
// ---------------------------------
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

#define U64_MAX     UINT64_MAX
#define U32_MAX     UINT32_MAX
#define U16_MAX     UINT16_MAX
#define  U8_MAX     UINT8_MAX
#define U64_MIN     0UL
#define U32_MIN     0U
#define U16_MIN     0U
#define  U8_MIN     0U
// ---------------------------------
// Signed int types.
// ---------------------------------
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

#define  I8_MAX     INT8_MAX
#define I16_MAX     INT16_MAX
#define I32_MAX     INT32_MAX
#define I64_MAX     INT64_MAX

#define  I8_MIN     INT8_MIN
#define I16_MIN     INT16_MIN
#define I32_MIN     INT32_MIN
#define I64_MIN     INT64_MIN
// ---------------------------------
// Floating point types
// ---------------------------------
typedef float       f32;
typedef double      f64;
// ---------------------------------
// Boolean types
// ---------------------------------
typedef _Bool       b8;
// ---------------------------------
// Static assertions.
// ---------------------------------
#if defined(K_CSTD) && (K_CSTD >= 2011)
#   if defined(__clang__) || defined(__GNUC__)
#       define STATIC_ASSERT _Static_assert
#   else
#       define STATIC_ASSERT static_assert
#   endif

STATIC_ASSERT(sizeof( u8) == 1, "Expected  u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof( i8) == 1, "Expected  i8 to be 1 byte.");
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
#   define K_INLINE __attribute__((always_inline)) inline
#   define K_NOINLINE __attribute__((noinline))
#   define K_DEPRECATED(message) __attribute__((deprecated(message)))
#   if defined(K_EXPORT)
#       define K_API __attribute__((visibility("default")))
#   else
#       define K_API
#   endif
#elif defined(_MSC_VER)
#   define K_INLINE __forceinline
#   define K_NOINLINE __declspec(noinline)
#   define K_DEPRECATED(message) __declspec(deprecated(message))
#   if defined(K_EXPORT)
#       define K_API __declspec(dllexport)
#   elif defined(K_IMPORT)
#       define K_API __declspec(dllimport)
#   else
#       define K_API
#   endif
#else
#   define K_API
#   define K_INLINE static inline
#   define K_NOINLINE
#   define K_DEPRECATED(message)
#endif
// ---------------------------------
// Helper Functions
// ---------------------------------
#define K_GIBI_BYTES(amount) ((amount) * 1024ULL * 1024ULL * 1024ULL)
#define K_MEBI_BYTES(amount) ((amount) * 1024ULL * 1024ULL)
#define K_KIBI_BYTES(amount) ((amount) * 1024ULL)

#define K_GIGA_BYTES(amount) ((amount) * 1000ULL * 1000ULL * 1000ULL)
#define K_MEGA_BYTES(amount) ((amount) * 1000ULL * 1000ULL)
#define K_KILO_BYTES(amount) ((amount) * 1000ULL)

#define K_SIZE_POINTER sizeof(void *)

typedef struct KFrame KFrame;
typedef struct KAllocator KAllocator;
typedef struct KCollection KCollection;
typedef struct KIterator KIterator;
typedef struct KString KString;

typedef struct {
    const u16 millis;
    const u16 year; const u8 month; const u8 day;
    const u8 hour; const u8 minute; const u8 second;
} KClock;

#endif
