#ifndef __TELEIOS_PROFILER__
#define __TELEIOS_PROFILER__

#include "teleios/defines.h"

void tl_profiler_begin(const char *name);
void tl_profiler_end(const char *name);
void tl_profiler_tick(const char *name);
u64 tl_profiler_time(const char *name);
u64 tl_profiler_ticks(const char *name);

#define TLPROFILER_TICK   tl_profiler_tick(__func__)
#define TLPROFILER_TICKS  tl_profiler_ticks(__func__)
#define TLPROFILER_MICROS tl_profiler_time(__func__)

#endif //__TELEIOS_PROFILER__
