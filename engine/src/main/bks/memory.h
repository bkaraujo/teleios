#ifndef __BK_STANDARD_MEMORY__
#define __BK_STANDARD_MEMORY__

#include "bks/defines.h"

void* bks_memory_alloc(u64 size);
void bks_memory_free(void *block);
void bks_memory_set(void *target, i32 value, u64 size);
void bks_memory_copy(void *target, const void *source, u64 size);

#endif
