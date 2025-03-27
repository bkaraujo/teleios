#ifndef __TELEIOS_CORE_MEMORY_OPERATOR__
#define __TELEIOS_CORE_MEMORY_OPERATOR__

#include "teleios/defines.h"
#include "teleios/core/memory/types.h"

void* tl_memory_alloc(TLMemoryArena *arena, u64 size, TLMemoryTag tag);
void tl_memory_set(void *block, i32 value, u64 size);
void tl_memory_copy(void *target, void *source, u64 size);

#endif //__TELEIOS_CORE_MEMORY_OPERATOR__
