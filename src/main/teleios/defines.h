#ifndef __TELEIOS_DEFINES__
#define __TELEIOS_DEFINES__

#define K_RUNTIME_FRAME_MAXIMUM 1

#include "libk/libk.h"

#define TL_ARR_LENGTH(a,t) (sizeof(a) / sizeof(t))
#define TL_ARR_SIZE(a,t) ((sizeof(a) / sizeof(t)) * sizeof(t))

typedef enum {
    TL_MEMORY_YAML,
    TL_MEMORY_MAXIMUM
} TLMemoryTag;


#endif // __TELEIOS_DEFINES__