#ifndef __TELEIOS_RUNTIME__
#define __TELEIOS_RUNTIME__

#include "teleios/defines.h"
#include "teleios/runtime/types.h"
#include "teleios/runtime/scene.h"
#include "teleios/runtime/engine.h"
#include "teleios/runtime/layer.h"

b8 tl_runtime_initialize(void);
b8 tl_runtime_terminate(void);

#endif // __TELEIOS_RUNTIME__