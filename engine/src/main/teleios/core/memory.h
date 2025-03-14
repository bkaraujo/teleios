#ifndef __TELEIOS_PLATFORM_MEMORY__
#define __TELEIOS_PLATFORM_MEMORY__

#include "teleios/defines.h"

TLMemoryArena* tl_memory_arena_create(u64 size);
void tl_memory_arena_destroy(TLMemoryArena* arena);
void tl_memory_arena_reset(TLMemoryArena* arena);

void* tl_memory_alloc(TLMemoryArena* arena, u64 size, TLMemoryTag tag);
void tl_memory_set(void *block, i32 value, u64 size);
void tl_memory_copy(void *target, void *source, u64 size);

#endif // __TELEIOS_PLATFORM_MEMORY__