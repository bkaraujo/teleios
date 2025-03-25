#ifndef __TELEIOS_GLOBALS__
#define __TELEIOS_GLOBALS__

#include "teleios/core/types.h"
#include "teleios/runtime/types.h"

typedef struct {
    TLString* yaml;

#if ! defined(TELEIOS_BUILD_RELEASE)
    u8 stack_index;
    u8 stack_maximum;
    TLStackFrame stack[10];
#endif

    struct {
        // Execution-wide arena
        TLMemoryArena* arena;

        struct {
            u64 allocated;
            u64 tagged_count[TL_MEMORY_MAXIMUM];
            u64 tagged_size[TL_MEMORY_MAXIMUM];
            TLMemoryArena *arenas[U8_MAX];
        } memory;

        struct {
            b8 vsync;
            b8 wireframe;
        } graphics;

        struct {
            TLString *title;
            void* handle;
            ivec2s size;
            ivec2s position;
            b8 visible;
            b8 maximized;
            b8 minimized;
            b8 focused;
            b8 hovered;
        } window;
    } platform;

    struct {
        TLString *version;
        TLMemoryArena *arena;

        struct {
            f64 step;
            u64 current;
            u32 per_second;
        } simulation;

        struct {
            TLString* name;

            TLMemoryArena *arena;

            struct {
                vec4s clear_color;
                i32 depth_function;
                i32 blend_equation;
                i32 blend_function_src;
                i32 blend_function_tgt;
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

                vec3s scale;
                vec3s rotation;
                vec3s position;
            } camera;

            b8 found;
            char prefix[U8_MAX];
        } scene;

        struct {
            u64 current;
            u32 per_second;
            TLMemoryArena *arena;
        } frame;
    } application;

} TLGlobal;

extern TLGlobal *global;

#endif //__TELEIOS_GLOBALS__
