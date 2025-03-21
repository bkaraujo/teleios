#ifndef __TELEIOS_CORE__
#define __TELEIOS_CORE__

#include "teleios/defines.h"
#include "teleios/core/types.h"

#include "teleios/core/logger.h"
#include "teleios/core/ulid.h"
#include "teleios/core/string.h"
#include "teleios/core/event.h"
#include "teleios/core/container.h"

#include "teleios/core/platform.h"
#include "teleios/core/graphics.h"
#include "teleios/core/serializer.h"

#include "teleios/core/time.h"
#include "teleios/core/memory.h"
#include "teleios/core/profiler.h"
#include "teleios/core/meta.h"

#include "teleios/core/libs.h"

typedef struct TLRuntime {
    struct TLGlobals {
        TLMemoryArena* permanent;
        TLMemoryArena* scene;
        TLMemoryArena* frame;
    } arenas;

    // Platform specifics
    struct TLPlatform {
        // Platform window state
        struct TLWindow {
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
        struct TLMemory {
            u64 allocated;
            u64 tagged_count[TL_MEMORY_MAXIMUM];
            u64 tagged_size[TL_MEMORY_MAXIMUM];
            TLMemoryArena* arenas[U8_MAX];
        } memory;
    } platform;

    struct {
        struct TLGraphics {
            b8 vsync;
            b8 wireframe;
        } graphics;

        struct TLSimulation {
            f64 step;
        } simulation;

        struct TLEcs {
            TLList *entities;
            TLList *components;
        } ecs;
    } engine;

#if ! defined(TELEIOS_BUILD_RELEASE)
    u8 stack_index;
    u8 stack_maximum;
    TLStackFrame stack[10];
#endif
} TLRuntime;

extern TLRuntime *runtime;

#endif // __TELEIOS_CORE__