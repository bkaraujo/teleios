#ifndef __TELEIOS_PLATFORM__TIME__
#define __TELEIOS_PLATFORM__TIME__

#include "teleios/defines.h"

void tl_time_clock(TLClock* clock);

u64 tl_time_epoch_millis(void);
u64 tl_time_epoch_micros(void);

#endif // __TELEIOS_PLATFORM__TIME__