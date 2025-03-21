#ifndef __TELEIOS_RUNTIME_TYPES__
#define __TELEIOS_RUNTIME_TYPES__

#include "teleios/defines.h"
#include "teleios/core/types.h"

// ---------------------------------
// Scene
// ---------------------------------
typedef struct {
    struct {
        vec4s clear_color;
        u32 depth_function;
        u32 blend_equations;
        u32 blend_function_src;
        u32 blend_function_tgt;
        b8 blend_enabled;
        b8 depth_enabled;
    } graphics;

    struct {
        struct {
            i8 left;
            i8 right;
            i8 up;
            i8 down;
            i8 near;
            i8 far;
        } rectangle;
    } camera;

    const char *entities[27];
    TLMemoryArena* arena;
} TLScene;

#endif //__TELEIOS_RUNTIME_TYPES__
