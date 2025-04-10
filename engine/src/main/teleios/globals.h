#ifndef __TELEIOS_GLOBALS__
#define __TELEIOS_GLOBALS__

#include "teleios/runtime.h"
#include "teleios/core/libs.h"
#include "teleios/runtime/libs.h"

typedef struct {
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
} TLInput;

typedef struct {
    TLString *yaml;
    TLString *rootfs;
    TLMap *properties;
    KAllocator* allocator;

    struct {
        struct {
            b8 vsync;
            b8 wireframe;
        } graphics;

        struct {
            TLString *title;
            void *handle;
            ivec2s size;
            ivec2s position;
            b8 visible;
            b8 maximized;
            b8 minimized;
            b8 focused;
            b8 hovered;
        } window;

        TLInput input;

        struct {
            lua_State *state;
        } script;
    } platform;

    struct {
        TLString *version;
        b8 running;
        b8 paused;

        struct {
            f64 step;
            u32 number;
            u16 overflow;
            u16 per_second;
        } simulation;

        struct {
            TLUlid *entity[TL_SCENE_MAX_ACTORS];
            struct {
                struct {
                    TLUlid *entity;
                    TLString *prefix;
                } yaml[TL_SCENE_MAX_ACTORS];
                struct {
                    TLUlid *entity;
                    TLString *name;
                } name[TL_SCENE_MAX_ACTORS];
                struct {
                    TLUlid *entity;
                    TLString *prefix;
                    TLString *path;
                } script[TL_SCENE_MAX_ACTORS * TL_SCENE_MAX_ACTOR_SCRIPTS];
            } components;
        } ecs;

        struct {
            TLString* name;
            KAllocator* allocator;

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
            KAllocator* allocator;
            u32 number;
            u16 overflow;
            u16 per_second;

            struct { TLInput input; } last;
            struct { TLInput input; } current;
        } frame;
    } application;

} TLGlobal;

extern TLGlobal *global;

#endif //__TELEIOS_GLOBALS__
