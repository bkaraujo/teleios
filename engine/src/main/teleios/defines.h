#ifndef __TELEIOS_DEFINES__
#define __TELEIOS_DEFINES__

#include <stddef.h>

// ---------------------------------
// Unsigned int types.
// ---------------------------------
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

#define U64_MAX 18446744073709551615UL
#define U32_MAX 4294967295U
#define U16_MAX 65535U
#define  U8_MAX 255U
#define U64_MIN 0UL
#define U32_MIN 0U
#define U16_MIN 0U
#define  U8_MIN 0U
// ---------------------------------
// Signed int types.
// ---------------------------------
typedef signed char         i8;
typedef signed short        i16;
typedef signed int          i32;
typedef signed long long    i64;

#define  I8_MAX             127
#define I16_MAX             32767
#define I32_MAX             2147483647
#define I64_MAX             9223372036854775807L
#define  I8_MIN             (-I8_MAX - 1)
#define I16_MIN             (-I16_MAX - 1)
#define I32_MIN             (-I32_MAX - 1)
#define I64_MIN             (-I64_MAX - 1)
// ---------------------------------
// Floating point types
// ---------------------------------
typedef float               f32;
typedef double              f64;
// ---------------------------------
// Boolean types
// ---------------------------------
typedef int                 b32;
typedef char                b8;

#define TRUE                1
#define FALSE               0
// ---------------------------------
// Static assertions.
// ---------------------------------
#if defined(__clang__) 
#   define static_assert _Static_assert
#elif defined(__GNUC__)
#   include <assert.h>
#else
#   define STATIC_ASSERT static_assert
#endif

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
    TL_MEMORY_CONTAINER_LIST,
    TL_MEMORY_CONTAINER_NODE,
    TL_MEMORY_CONTAINER_ITERATOR,
    TL_MEMORY_WINDOW,
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
// Container
// ---------------------------------
typedef struct TLList TLList;
typedef struct TLIterator TLIterator;
// ---------------------------------
// Scene
// ---------------------------------
typedef struct {
    struct {
        vec4s clear_color;
    } graphics;

    TLList *entities;
} TLScene;
// ---------------------------------
// Globals
// ---------------------------------
typedef struct {
    struct {
        struct {
            u32 width;
            u32 height;
            const char *title;
            void* handle;
            b8 maximized;
            b8 minimized;
            b8 focused;
            b8 hovered;
        } window;

        struct {
            u64 allocated;
            u64 tagged_count[TL_MEMORY_MAXIMUM];
            u64 tagged_size[TL_MEMORY_MAXIMUM];
            TLMemoryArena* arenas[U8_MAX];
        } memory;
    } platform;

    struct {
        b8 vsync;
        b8 wireframe;
    } graphics;

    struct {
        TLList *entities;
        TLList *components;
    } ecs; 

    struct {
        f64 step;
    } simulation;

    TLScene* scene;
    TLMemoryArena* arena_frame;
    TLMemoryArena* arena_persistent;
} TLRuntime;

extern TLRuntime *runtime;

#endif // __TELEIOS_DEFINES__