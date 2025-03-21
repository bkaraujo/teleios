#ifndef __TELEIOS_TYPES__
#define __TELEIOS_TYPES__

#include "teleios/core/types.h"
#include "teleios/runtime/types.h"

typedef struct TLCore {
    struct {
        TLMemoryArena* permanent;
        TLMemoryArena* scene;
        TLMemoryArena* frame;
    } arenas;

    // Platform window state
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
    // Platform allocated memory
    struct {
        u64 allocated;
        u64 tagged_count[TL_MEMORY_MAXIMUM];
        u64 tagged_size[TL_MEMORY_MAXIMUM];
        TLMemoryArena* arenas[U8_MAX];
    } memory;

    struct {
        b8 vsync;
        b8 wireframe;
    } graphics;

    struct {
        f64 step;
    } simulation;

    struct {
        TLList *entities;
        TLList *components;
    } ecs;

#if ! defined(TELEIOS_BUILD_RELEASE)
    u8 stack_index;
    u8 stack_maximum;
    TLStackFrame stack[10];
#endif
} TLGlobal;

extern TLGlobal *global;

#endif //__TELEIOS_TYPES__
