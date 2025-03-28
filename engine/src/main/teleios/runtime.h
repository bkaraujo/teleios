#ifndef __TELEIOS_RUNTIME__
#define __TELEIOS_RUNTIME__

#include "teleios/defines.h"
#include "teleios/runtime/memory.h"
#include "teleios/runtime/container.h"
#include "teleios/runtime/serializer.h"
#include "teleios/runtime/script.h"
#include "teleios/runtime/input.h"
#include "teleios/runtime/ulid.h"
#include "teleios/runtime/string.h"

b8 tl_runtime_initialize(void);
b8 tl_runtime_terminate(void);

#endif // __TELEIOS_RUNTIME__