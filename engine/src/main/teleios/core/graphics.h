#ifndef __TELEIOS_CORE_GRAPHICS__
#define __TELEIOS_CORE_GRAPHICS__

#include "teleios/defines.h"
#include "teleios/core/graphics/types.h"
#include "teleios/core/graphics/geometry.h"
#include "teleios/core/graphics/shader.h"
#include "teleios/core/graphics/texture.h"
#include "teleios/core/graphics/primitive.h"

b8 tl_graphics_initialize(void);
b8 tl_graphics_terminate(void);

#endif // __TELEIOS_CORE_GRAPHICS__