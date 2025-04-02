#ifndef __TELEIOS_DEFINES__
#define __TELEIOS_DEFINES__

#ifdef __STDC__
#   define PREDEF_STANDARD_C_1989
#   ifdef __STDC_VERSION__
#       if (__STDC_VERSION__ >= 199409L)
#           define C94
#       endif
#       if (__STDC_VERSION__ >= 199901L)
#           define C99
#       endif
#       if (__STDC_VERSION__ >= 201112L)
#           define C11
#           include <stdatomic.h>
#           include <assert.h>
#           include <stdalign.h>
#       endif
#       if (__STDC_VERSION__ >= 201710L)
#           define C18
#       endif
#   endif
#endif

#include <stdint.h>
#include <stdbool.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   define TLPLATFORM_WINDOWS 1
#   ifndef _WIN64
#       error "64-bit is required on Windows!"
#   endif
#   include <windows.h>
#endif

#if defined(_POSIX_VERSION)
#   define TLPLATFORM_POSIX 1
#endif

#if defined(__linux__) || defined(__gnu_linux__)
#   define TLPLATFORM_LINUX 1
#   if defined(__ANDROID__)
#       define TLPLATFORM_ANDROID 1
#   endif
#endif

#if defined(__unix__)
#   define TLPLATFORM_UNIX 1
#   include <unistd.h>
#   if defined(_XOPEN_VERSION)
#       if (_XOPEN_VERSION >= 3)
#           define XOPEN_1989
#       endif
#       if (_XOPEN_VERSION >= 4)
#           define XOPEN_1992
#       endif
#       if (_XOPEN_VERSION >= 4) && defined(_XOPEN_UNIX)
#           define XOPEN_1995
#       endif
#       if (_XOPEN_VERSION >= 500)
#           define XOPEN_1998
#       endif
#       if (_XOPEN_VERSION >= 600)
#           define XOPEN_2003
#       endif
#       if (_XOPEN_VERSION >= 700)
#           define XOPEN_2008
#       endif
#   endif
#endif

#if __APPLE__
#   define TLPLATFORM_APPLE 1
#   include <TargetConditionals.h>
#   if TARGET_IPHONE_SIMULATOR
#       define TLPLATFORM_IOS 1
#       define TLPLATFORM_IOS_SIMULATOR 1
#   elif TARGET_OS_IPHONE
#       define TLPLATFORM_IOS 1
#   elif TARGET_OS_MAC
#   else
#       error "Unknown Apple platform"
#   endif
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
typedef float               f32;
typedef double              f64;
// ---------------------------------
// Boolean types
// ---------------------------------
typedef _Bool               b8;
// ---------------------------------
// Static assertions.
// ---------------------------------
#ifdef C11
static_assert(sizeof( u8) == 1, "Expected  u8 to be 1 byte.");
static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

static_assert(sizeof( i8) == 1, "Expected  i8 to be 1 byte.");
static_assert(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
static_assert(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
static_assert(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");
#endif // C11

#include "cglm/types-struct.h"
// ---------------------------------
// Compiler specifics
// ---------------------------------
#if defined(__clang__) || defined(__GNUC__)
#   define TL_INLINE __attribute__((always_inline)) inline
#   define TL_NOINLINE __attribute__((noinline))
#   define TL_DEPRECATED(message) __attribute__((deprecated(message)))
#elif defined(_MSC_VER)
#   define TL_INLINE __forceinline
#   define TL_NOINLINE __declspec(noinline)
#   define TL_DEPRECATED(message) __declspec(deprecated(message))
#else
#   define TL_INLINE static inline
#   define TL_NOINLINE
#endif
// ---------------------------------
// Helper Functions
// ---------------------------------
#define TL_GIBI_BYTES(amount) ((amount) * 1024ULL * 1024ULL * 1024ULL)
#define TL_MEBI_BYTES(amount) ((amount) * 1024ULL * 1024ULL)
#define TL_KIBI_BYTES(amount) ((amount) * 1024ULL)

#define TL_GIGA_BYTES(amount) ((amount) * 1000ULL * 1000ULL * 1000ULL)
#define TL_MEGA_BYTES(amount) ((amount) * 1000ULL * 1000ULL)
#define TL_KILO_BYTES(amount) ((amount) * 1000ULL)

#define TL_ARR_LENGTH(a,t) (sizeof(a) / sizeof(t))
#define TL_ARR_SIZE(a,t) ((sizeof(a) / sizeof(t)) * sizeof(t))
#define TL_CREATE_CHAR(name, size) char name[size]; tl_memory_set(name, 0 , size);
// ---------------------------------
// Globals
// ---------------------------------

typedef struct TLList TLStack;
typedef struct TLList TLList;
typedef struct TLIterator TLIterator;
typedef struct TLString TLString;
typedef struct TLUlid TLUlid;
typedef struct TLUlidGenerator TLUlidGenerator;
typedef struct TLMemoryArena TLMemoryArena;

#if ! defined(TELEIOS_BUILD_RELEASE)
typedef struct TLStackFrame {
    u64 timestamp;
    char filename[100];
    char function[100];
    char arguments[1024];
    u32 lineno;
} TLStackFrame;
#endif

#define TL_YAML_PROPERTY_MAX_SIZE 1024
#define TL_STACK_STRING_SIZE 100
#define TL_STACK_ARGUMENTS_SIZE 1024

#endif // __TELEIOS_DEFINES__