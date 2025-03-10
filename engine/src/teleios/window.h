#ifndef __TELEIOS_WINDOW__
#define __TELEIOS_WINDOW__

#include "teleios/defines.h"

typedef struct TLWindow TLWindow;

TLWindow* tl_window_create(u32 width, u32 heigth, const char *title);
void tl_window_destroy(TLWindow *window);

#endif // __TELEIOS_WINDOW__