#ifndef __LIBK_TIME__
#define __LIBK_TIME__

#include "libk/defines.h"

void k_time_clock(KClock* clock);
u64 k_time_epoch_millis(void);
u64 k_time_epoch_micros(void);

#endif