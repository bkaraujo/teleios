#ifndef __TELEIOS_RUNTIME__
#define __TELEIOS_RUNTIME__

#include "teleios/defines.h"

typedef enum {
    TL_RUNTIME_MEMORY_MAXIMUM
} TLRuntimeMemoryTag;

typedef struct {
    KAllocator *allocator;
    KCollection *yaml;
} TLRuntime;

TLRuntime* tl_runtime_get();

b8 tl_runtime_initialize(const char *yaml);
b8 tl_runtime_terminate(void);

#endif