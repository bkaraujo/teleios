#ifndef __TELEIOS_CORE_TYPES__
#define __TELEIOS_CORE_TYPES__

#include "teleios/defines.h"

typedef struct TLUlid TLUlid;
typedef struct TLUlidGenerator TLUlidGenerator;
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
    TL_MEMORY_ULID,
    TL_MEMORY_PROFILER,
    TL_MEMORY_SCENE,
    TL_MEMORY_MAXIMUM
} TLMemoryTag;

typedef struct TLMemoryArena TLMemoryArena;
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
// Event Handling
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
typedef struct TLStack TLStack;
typedef struct TLList TLList;
typedef struct TLIterator TLIterator;
typedef struct TLString TLString;
// ---------------------------------
// Meta Programming
// ---------------------------------
#if ! defined(TELEIOS_BUILD_RELEASE)
typedef struct {
    u64 timestamp;
    char filename[100];
    char function[100];
    char arguments[1024];
    u32 lineno;
} TLStackFrame;
#endif

#endif // __TELEIOS_CORE_TYPES__
