#ifndef __TELEIOS_GRAPHICS__
#define __TELEIOS_GRAPHICS__

#include "teleios/defines.h"

b8 tl_graphics_initialize(void);
b8 tl_graphics_terminate(void);

TL_API void tl_graphics_submit_sync(void (*func)(void));
TL_API void tl_graphics_submit_async(void (*func)(void));

TL_API void tl_graphics_submit_sync_args(void (*func)(void*), void* args);
TL_API void tl_graphics_submit_async_args(void (*func)(void*), void* args);

#endif