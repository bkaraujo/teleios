#ifndef __TELEIOS_CORE_PLATFORM__
#define __TELEIOS_CORE_PLATFORM__

#include "teleios/defines.h"

void* tl_platform_memory_alloc(u64 size);
void tl_platform_memory_free(void *block);
void tl_platform_memory_set(void *block, i32 value, u64 size);
void tl_platform_memory_copy(void *target, const void *source, u64 size);

b8 tl_platform_initialize(void);
b8 tl_platform_terminate(void);

#endif // __TELEIOS_CORE_PLATFORM__