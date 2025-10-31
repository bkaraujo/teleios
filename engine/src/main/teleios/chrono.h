#ifndef __TELEIOS_CHRONO__
#define __TELEIOS_CHRONO__

#include "teleios/defines.h"

typedef struct {
    u16 millis;
    u16 year; u8 month; u8 day;
    u8 hour; u8 minute; u8 second;
} TLDateTime;

void tl_time_clock(TLDateTime* clock);
u64 tl_time_epoch_millis(void);
u64 tl_time_epoch_micros(void);

#endif