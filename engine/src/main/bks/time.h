#ifndef __BK_STANDARD_TIME__
#define __BK_STANDARD_TIME__

#include "bks/defines.h"

typedef struct {
    u16 millis;
    u16 year; u8 month; u8 day;
    u8 hour; u8 minute; u8 second;
} BKSClock;

void bks_time_clock(BKSClock* clock);
u64 bks_time_epoch_millis(void);
u64 bks_time_epoch_micros(void);

#endif