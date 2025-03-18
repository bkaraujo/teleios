#ifndef __TELEIOS_PROFILER__
#define __TELEIOS_PROFILER__

#include "teleios/defines.h"

void tl_profiler_begin(const char *name);
void tl_profiler_tick(const char *name);
u64 tl_profiler_time(const char *name);
u64 tl_profiler_count(const char *name);
void tl_profiler_end(const char *name);

#endif //__TELEIOS_PROFILER__
