#ifndef __TELEIOS_PROFILER_TYPES__
#define __TELEIOS_PROFILER_TYPES__

#include "teleios/defines.h"


#if ! defined(TELEIOS_FRAME_MAXIMUM)
#   define TELEIOS_FRAME_MAXIMUM 128
#endif

#if ! defined(TL_PROFILER_FRAME_ARGUMENTS_SIZE)
#   define TL_PROFILER_FRAME_ARGUMENTS_SIZE 1024
#endif

typedef struct  {
    const char* filename;
    const char* function;
    char arguments[TL_PROFILER_FRAME_ARGUMENTS_SIZE];
    u32 lineno;
} TLStackFrame;

struct  TLStackTrace {
    TLStackFrame frames[TELEIOS_FRAME_MAXIMUM];
    u16 depth;
} ;

#endif