#ifndef __TELEIOS_CORE_TYPES__
#define __TELEIOS_CORE_TYPES__

#include "teleios/defines.h"

typedef struct TLList TLStack;
typedef struct TLList TLList;
typedef struct TLIterator TLIterator;
typedef struct TLString TLString;
typedef struct TLUlid TLUlid;
typedef struct TLUlidGenerator TLUlidGenerator;
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

#endif // __TELEIOS_CORE_TYPES__
