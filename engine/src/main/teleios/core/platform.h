#ifndef __TELEIOS_PLATFORM__
#define __TELEIOS_PLATFORM__

#include "teleios/defines.h"

void* tl_platform_memory_alloc(u64 size);
void tl_platform_memory_free(void *block);
void tl_platform_memory_set(void *block, i32 value, u64 size);
void tl_platform_memory_copy(void *target, const void *source, u64 size);

typedef struct {
    u16 millis;
    u16 year; u8 month; u8 day;
    u8 hour; u8 minute; u8 second;
} TLClock;

void tl_time_clock(TLClock* clock);
u64 tl_time_epoch_millis(void);
u64 tl_time_epoch_micros(void);

b8 tl_platform_initialize(void);
b8 tl_platform_terminate(void);

#endif // __TELEIOS_PLATFORM__