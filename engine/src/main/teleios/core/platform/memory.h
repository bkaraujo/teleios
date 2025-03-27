#ifndef __TELEIOS_CORE_PLATFORM_MEMORY__
#define __TELEIOS_CORE_PLATFORM_MEMORY__

#include "teleios/defines.h"

void* tl_platform_memory_alloc(u64 size);
void tl_platform_memory_free(void *block);
void tl_platform_memory_set(void *block, i32 value, u64 size);
void tl_platform_memory_copy(void *target, const void *source, u64 size);

#endif //__TELEIOS_CORE_PLATFORM_MEMORY__
