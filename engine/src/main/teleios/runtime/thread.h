#ifndef __TELEIOS_RUNTIME_THREAD__
#define __TELEIOS_RUNTIME_THREAD__

#include "teleios/defines.h"

typedef void (*PFN_task)();
void tl_thread_fire_and_wait(PFN_task task, u64 timeout);
void tl_thread_fire_and_forget(PFN_task task);

b8 tl_thread_initialize(void);
b8 tl_thread_terminate(void);

#endif //__TELEIOS_RUNTIME_THREAD__
