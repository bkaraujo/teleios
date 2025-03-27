#ifndef __TELEIOS_CORE_PLATFORM_WINDOW__
#define __TELEIOS_CORE_PLATFORM_WINDOW__

#include "teleios/defines.h"
#include "teleios/core/types.h"

void tl_window_create();
void tl_window_show();
void tl_window_hide();
void tl_window_set_title(TLString *title);
void tl_window_destroy();

#endif //__TELEIOS_CORE_PLATFORM_WINDOW__
