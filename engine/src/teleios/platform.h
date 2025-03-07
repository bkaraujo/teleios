#ifndef __TELEIOS_PLATFORM__
#define __TELEIOS_PLATFORM__

#include "teleios/defines.h"
#include "teleios/logger.h"

b8 tl_platform_initialize(void);
b8 tl_platform_terminate(void);

void tl_platform_clock(TLClock* clock);

void tl_platform_stdout(TLLogLevel level, const char* message);

void* tl_platform_memory_alloc(u64 size, TLMemoryTag tag);
void tl_platform_memory_free(void* block);
void tl_platform_memory_set(void* block, i32 value, u64 size);
void tl_platform_memory_copy(void* target, void* source, u64 size);

#endif // __TELEIOS_PLATFORM__