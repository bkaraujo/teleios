#ifndef __TELEIOS_GLOBALS__
#define __TELEIOS_GLOBALS__

#include "teleios/core/types.h"
#include "teleios/runtime/types.h"
#include "teleios/core/libs.h"

#include "teleios/core/memory/types.h"
#define STACK_SIZE_MAXIMUM 10

typedef struct {
    TLString* yaml;

#if ! defined(TELEIOS_BUILD_RELEASE)
    u8 stack_index;
    u8 stack_maximum;
    TLStackFrame stack[STACK_SIZE_MAXIMUM];
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

        struct {
            struct {
                b8 key[GLFW_KEY_LAST + 1];
            } keyboard;
            struct {
                b8 button[GLFW_MOUSE_BUTTON_LAST + 1];
                u32 position_x;
                u32 position_y;
                u8 scroll_x;
                u8 scroll_y;
                b8 hoover;
            } cursor;
        } input;
    } platform;

    struct {
        TLString *version;
        TLMemoryArena *arena;

        struct {
            f64 step;
            u32 current;
            u16 overflow;
            u16 per_second;
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

            TLList *layers;

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
            TLMemoryArena *arena;
            u32 current;
            u16 overflow;
            u16 per_second;

            struct {
                struct {
                    struct {
                        b8 key[GLFW_KEY_LAST + 1];
                    } keyboard;
                    struct {
                        b8 button[GLFW_MOUSE_BUTTON_LAST + 1];
                        u32 position_x;
                        u32 position_y;
                        u8 scroll_x;
                        u8 scroll_y;
                        b8 hoover;
                    } cursor;
                } input;
            } last;
        } frame;
    } application;

} TLGlobal;

extern TLGlobal *global;

#endif //__TELEIOS_GLOBALS__
