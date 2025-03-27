#ifndef __TELEIOS_CORE_TYPES__
#define __TELEIOS_CORE_TYPES__

#include "teleios/defines.h"

typedef struct TLList TLStack;
typedef struct TLList TLList;
typedef struct TLIterator TLIterator;
typedef struct TLString TLString;
typedef struct TLUlid TLUlid;
typedef struct TLUlidGenerator TLUlidGenerator;
typedef struct TLMemoryArena TLMemoryArena;

typedef struct {
    u16 millis;
    u16 year; u8 month; u8 day;
    u8 hour; u8 minute; u8 second;
} TLClock;

#if ! defined(TELEIOS_BUILD_RELEASE)
typedef struct TLStackFrame {
    u64 timestamp;
    char filename[100];
    char function[100];
    char arguments[1024];
    u32 lineno;
} TLStackFrame;
#endif

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

#endif // __TELEIOS_CORE_TYPES__
