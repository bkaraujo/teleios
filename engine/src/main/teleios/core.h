#ifndef __TELEIOS_CORE__
#define __TELEIOS_CORE__

#include "teleios/defines.h"

#include "teleios/core/logger.h"
#include "teleios/core/string.h"
#include "teleios/core/event.h"
#include "teleios/core/container.h"

#include "teleios/core/platform.h"
#include "teleios/core/graphics.h"

#include "teleios/core/time.h"
#include "teleios/core/memory.h"

b8 tl_core_initialize(void);
b8 tl_core_terminate(void);

#endif // __TELEIOS_CORE__