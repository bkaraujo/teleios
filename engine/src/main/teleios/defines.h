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

#define TRUE                true
#define FALSE               false
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
#   define TLINLINE __attribute__((always_inline)) inline
#   define TLNOINLINE __attribute__((noinline))
#   define TLDEPRECATED(message) __attribute__((deprecated(message)))
#elif defined(_MSC_VER)
#   define TLINLINE __forceinline
#   define TLNOINLINE __declspec(noinline)
#   define TLDEPRECATED(message) __declspec(deprecated(message))
#else
#   define TLINLINE static inline
#   define TLNOINLINE
#endif

#if defined(__GNUC__)
#   define TLALLOCA(s) __builtin_alloca(s)
#   define TLMALLOC(s) __builtin_malloc(s)
#   define TLFREE(p) __builtin_free(p)
#   define TLMEMSET(p,v,s) __builtin_memset(p, v, s)
#   define TLMEMCPY(p,s,sx) __builtin_memcpy(p, s, sx)
#elif defined(_MSC_VER)
#   define TLALLOCA(s) alloca(s)
#   define TLMALLOC(s) malloc(s)
#   define TLFREE(p) free(p)
#   define TLMEMSET(p,v,s) memset(p, v, s)
#   define TLMEMCPY(p,s,sx) memcpy(p, s, sx)
#else
#   error "Unknown compiler!"
#endif
// ---------------------------------
// Helper Functions
// ---------------------------------
#define TLGIBIBYTES(amount) ((amount) * 1024ULL * 1024ULL * 1024ULL)
#define TLMEBIBYTES(amount) ((amount) * 1024ULL * 1024ULL)
#define TLKIBIBYTES(amount) ((amount) * 1024ULL)

#define TLGIGABYTES(amount) ((amount) * 1000ULL * 1000ULL * 1000ULL)
#define TLMEGABYTES(amount) ((amount) * 1000ULL * 1000ULL)
#define TLKILOBYTES(amount) ((amount) * 1000ULL)
// ---------------------------------
// Memmory
// ---------------------------------
typedef enum {
    TL_MEMORY_BLOCK,
    TL_MEMORY_SERIALIZER,
    TL_MEMORY_CONTAINER_STACK,
    TL_MEMORY_CONTAINER_LIST,
    TL_MEMORY_CONTAINER_NODE,
    TL_MEMORY_CONTAINER_ITERATOR,
    TL_MEMORY_STRING,
    TL_MEMORY_PROFILER,
    TL_MEMORY_MAXIMUM
} TLMemoryTag;

typedef struct TLMemoryArena TLMemoryArena;
// ---------------------------------
// Time
// ---------------------------------
typedef struct {
    u16 millis;
    u16 year; u8 month; u8 day;
    u8 hour; u8 minute; u8 second;
} TLClock;
// ---------------------------------
// Graphics
// ---------------------------------
typedef enum {
    TL_VIDEO_RESOLUTION_SD  = 480,
    TL_VIDEO_RESOLUTION_HD  = 720,
    TL_VIDEO_RESOLUTION_FHD = 1080,
    TL_VIDEO_RESOLUTION_QHD = 1440,
    TL_VIDEO_RESOLUTION_UHD = 2160
} TLVideoResolution;
// ---------------------------------
// Container
// ---------------------------------
typedef struct TLStack TLStack;
typedef struct TLList TLList;
typedef struct TLIterator TLIterator;
typedef struct TLString TLString;
// ---------------------------------
// Scene
// ---------------------------------
typedef struct {
    struct {
        vec4s clear_color;
        u32 depth_function;
        u32 blend_equations;
        u32 blend_function_src;
        u32 blend_function_tgt;
        b8 blend_enabled;
        b8 depth_enabled;
    } graphics;

    struct {
        struct {
            i8 left;
            i8 right;
            i8 up;
            i8 down;
            i8 near;
            i8 far;
        } rectangle;
    } camera;

    const char *entities[27];
    TLMemoryArena* arena;
} TLScene;
// ---------------------------------
// Globals
// ---------------------------------
typedef enum {
    TL_EVENT_WINDOW_CREATED,
    TL_EVENT_WINDOW_RESIZED,
    TL_EVENT_WINDOW_DESTROYED,
    TL_EVENT_WINDOW_MOVED,
    TL_EVENT_WINDOW_MINIMIZED,
    TL_EVENT_WINDOW_MAXIMIZED,
    TL_EVENT_WINDOW_RESTORED,
    TL_EVENT_WINDOW_FOCUS_GAINED,
    TL_EVENT_WINDOW_FOCUS_LOST,

    TL_EVENT_MAXIMUM
} TLEventCodes;

typedef union {
    i64 i64[2];
    u64 u64[2];
    f64 f64[2];

    i32 i32[4];
    u32 u32[4];
    f32 f32[4];

    i16 i16[8];
    u16 u16[8];

    i8 i8[16];
    u8 u8[16];
} TLEvent;

typedef enum {
    TL_EVENT_CONSUMED       = 10,
    TL_EVENT_NOT_CONSUMED   = 11,
} TLEventStatus;

#if ! defined(TELEIOS_BUILD_RELEASE)
typedef struct {
    u64 timestamp;
    char filename[100];
    char function[100];
    char arguments[1024];
    u32 lineno;
} TLStackFrame;
#endif

typedef struct {
    struct {
        TLVideoResolution resolution;
        TLString *title;
    } window;

    struct {
        u8 step;
    } simulation;
} TLApplication;

typedef struct TLRuntime {
    struct TLGlobals {
        TLMemoryArena* frame;
        TLMemoryArena* permanent;
    } arenas;

    // Platform specifics
    struct TLPlatform {
        // Platform window state
        struct TLWindow {
            TLString *title;
            void* handle;
            ivec2s size;
            ivec2s position;
            b8 visible;
            b8 maximized;
            b8 minimized;
            b8 focused;
            b8 hovered;
        } window;
        // Platform allocated memory
        struct TLMemory {
            u64 allocated;
            u64 tagged_count[TL_MEMORY_MAXIMUM];
            u64 tagged_size[TL_MEMORY_MAXIMUM];
            TLMemoryArena* arenas[U8_MAX];
        } memory;
    } platform;

    struct {
        struct TLGraphics {
            b8 vsync;
            b8 wireframe;
        } graphics;

        struct TLSimulation {
            f64 step;
        } simulation;

        struct TLEcs {
            TLList *entities;
            TLList *components;
        } ecs;
    } engine;

    struct {

        TLScene* scene;
    } application;

#if ! defined(TELEIOS_BUILD_RELEASE)
    u8 stack_size;
    u8 stack_maximum;
    TLStackFrame stack[10];
#endif
} TLRuntime;

extern TLRuntime *runtime;

#endif // __TELEIOS_DEFINES__