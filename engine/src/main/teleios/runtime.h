#ifndef __TELEIOS_RUNTIME__
#define __TELEIOS_RUNTIME__

#include "teleios/defines.h"
#include "teleios/runtime/memory.h"
#include "teleios/runtime/thread.h"
#include "teleios/runtime/container.h"
#include "teleios/runtime/script.h"
#include "teleios/runtime/input.h"
#include "teleios/runtime/ulid.h"
#include "teleios/runtime/filesystem.h"

b8 tl_runtime_initialize(void);
b8 tl_runtime_terminate(void);

#endif // __TELEIOS_RUNTIME__