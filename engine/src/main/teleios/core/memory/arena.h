#ifndef __TELEIOS_CORE_MEMORY_ARENA__
#define __TELEIOS_CORE_MEMORY_ARENA__

#include "teleios/defines.h"
#include "teleios/core/types.h"

TLMemoryArena* tl_memory_arena_create(u64 size);
void tl_memory_arena_destroy(TLMemoryArena *arena);
void tl_memory_arena_reset(TLMemoryArena *arena);

#endif //__TELEIOS_CORE_MEMORY_ARENA__
