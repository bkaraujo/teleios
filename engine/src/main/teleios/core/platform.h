#ifndef __TELEIOS_PLATFORM__
#define __TELEIOS_PLATFORM__

#include "teleios/defines.h"
#include "teleios/core/platform/input.h"
#include "teleios/core/platform/audio.h"
#include "teleios/core/platform/thread.h"
#include "teleios/core/platform/window.h"
#include "teleios/core/platform/network.h"
#include "teleios/core/platform/filesystem.h"
#include "teleios/core/platform/time.h"

b8 tl_platform_initialize(void);
b8 tl_platform_terminate(void);

#endif // __TELEIOS_PLATFORM__