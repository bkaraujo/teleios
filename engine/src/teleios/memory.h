#ifndef __TELEIOS_PLATFORM_MEMORY__
#define __TELEIOS_PLATFORM_MEMORY__

#include "teleios/defines.h"

void *tl_memory_alloc(u64 size, TLMemoryTag tag);
void tl_memory_free(void *block);
void tl_memory_set(void *block, i32 value, u64 size);
void tl_memory_copy(void *target, void *source, u64 size);

#endif // __TELEIOS_PLATFORM_MEMORY__