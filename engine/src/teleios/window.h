#ifndef __TELEIOS_WINDOW__
#define __TELEIOS_WINDOW__

#include "teleios/defines.h"

typedef struct {
    b8 minimized;
    b8 maximized;
    b8 focused;
    b8 hovered;
} TLWindow;

#endif // __TELEIOS_WINDOW__