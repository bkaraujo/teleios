#ifndef __TELEIOS_RUNTIME_THREAD__
#define __TELEIOS_RUNTIME_THREAD__

#include "teleios/defines.h"

b8 tl_thread_initialize(void);
b8 tl_thread_terminate(void);

void tl_thread_fire_and_wait(void* function, u64 timeout);
void tl_thread_fire_and_forget(void* function);

#endif //__TELEIOS_RUNTIME_THREAD__
