#ifndef __TELEIOS_CORE_GRAPHICS__
#define __TELEIOS_CORE_GRAPHICS__

#include "teleios/defines.h"

typedef enum {
    TL_VIDEO_RESOLUTION_SD  = 480,
    TL_VIDEO_RESOLUTION_HD  = 720,
    TL_VIDEO_RESOLUTION_FHD = 1080,
    TL_VIDEO_RESOLUTION_QHD = 1440,
    TL_VIDEO_RESOLUTION_UHD = 2160
} TLVideoResolution;


b8 tl_graphics_initialize(void);
b8 tl_graphics_terminate(void);

#endif // __TELEIOS_CORE_GRAPHICS__