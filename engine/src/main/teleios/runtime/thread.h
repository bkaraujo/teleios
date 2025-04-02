#ifndef __TELEIOS_RUNTIME_THREAD__
#define __TELEIOS_RUNTIME_THREAD__

#include "teleios/defines.h"

typedef struct TLThreadPool TLThreadPool;

TLThreadPool tl_thread_pool_create(TLMemoryArena *arena);
void tl_thread_pool_destroy(const TLThreadPool *pool);

typedef void (*PFN_task)();
void tl_thread_fire_and_wait(PFN_task task, u64 timeout);
void tl_thread_fire_and_forget(PFN_task task);

#endif //__TELEIOS_RUNTIME_THREAD__
