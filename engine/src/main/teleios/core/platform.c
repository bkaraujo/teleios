#include "teleios/core.h"
#include "teleios/global.h"

// ########################################################
//                    MEMORY FUNCTIONS
// ########################################################
typedef struct {
    u64 index;
    char *payload;
} TLMemoryPage;

struct TLMemoryArena {
    u64 allocated;
    u64 tagged_count[TL_MEMORY_MAXIMUM];
    u64 tagged_size[TL_MEMORY_MAXIMUM];
    u64 page_size;
    TLMemoryPage page[U8_MAX];
};

static const char *tl_memory_name(const TLMemoryTag tag) {
    TLSTACKPUSHA("%d", tag)
    switch (tag) {
        case TL_MEMORY_BLOCK                : TLSTACKPOPV("TL_MEMORY_BLOCK")
        case TL_MEMORY_SERIALIZER           : TLSTACKPOPV("TL_MEMORY_SERIALIZER")
        case TL_MEMORY_CONTAINER_LIST       : TLSTACKPOPV("TL_MEMORY_CONTAINER_LIST")
        case TL_MEMORY_CONTAINER_STACK      : TLSTACKPOPV("TL_MEMORY_CONTAINER_STACK")
        case TL_MEMORY_CONTAINER_NODE       : TLSTACKPOPV("TL_MEMORY_CONTAINER_NODE")
        case TL_MEMORY_CONTAINER_ITERATOR   : TLSTACKPOPV("TL_MEMORY_CONTAINER_ITERATOR")
        case TL_MEMORY_STRING               : TLSTACKPOPV("TL_MEMORY_STRING")
        case TL_MEMORY_PROFILER             : TLSTACKPOPV("TL_MEMORY_PROFILER")
        case TL_MEMORY_MAXIMUM              : TLSTACKPOPV("TL_MEMORY_MAXIMUM")
    }

    TLSTACKPOPV("???")
}

TLMemoryArena* tl_memory_arena_create(const u64 size) {
    TLSTACKPUSHA("%d", size)
    // ----------------------------------------------------------
    // Create the memory arena
    // ----------------------------------------------------------
    TLMemoryArena* arena = TLMALLOC(sizeof(TLMemoryArena));
    if (arena == NULL) TLFATAL("Failed to allocate TLMemoryArena");
    TLMEMSET(arena, 0, sizeof(TLMemoryArena));
    arena->page_size = size;
    // ----------------------------------------------------------
    // Keep track of the created arena
    // ----------------------------------------------------------
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (global->memory.arenas[i] == NULL) {
            global->memory.arenas[i] = arena;
            TLVERBOSE("TLMemoryArena 0x%p created with page size of %d bytes", arena, arena->page_size)
            TLSTACKPOPV(arena)
        }
    }

    TLFATAL("Failed to allocate TLMemoryArena");
}

TLINLINE static u8 tl_memory_arena_get_index(TLMemoryArena* arena) {
    TLSTACKPUSHA("0x%p", arena)

    if (arena == NULL) {
        TLWARN("TLMemoryArena is NULL")
        TLSTACKPOPV(FALSE)
    }

    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (global->memory.arenas[i] == NULL) continue;
        if (global->memory.arenas[i] == arena) {
            TLSTACKPOPV(i)
        }
    }

    TLFATAL("TLMemoryArena 0x%p not found", arena)
}

TLINLINE static void tl_memory_arena_do_destroy(const u8 index) {
    TLSTACKPUSHA("%d", index)
    TLMemoryArena *arena = global->memory.arenas[index];
    for (u8 i = 0 ; i < TLARRSIZE(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload != NULL) {
            TLVERBOSE("TLMemoryArena 0x%p releasing page %d", arena, i)
            TLFREE(arena->page[i].payload);
            arena->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
        if (global->memory.arenas[index]->tagged_size[i] != 0) {
            TLVERBOSE("TLMemoryArena 0x%p at %-30s: [%03d] %llu bytes", arena, tl_memory_name(i), arena->tagged_count[i], arena->tagged_size[i]);
        }
    }

    TLFREE(arena);
    global->memory.arenas[index] = NULL;

    TLSTACKPOP
}

void tl_memory_arena_reset(TLMemoryArena* arena) {
    TLSTACKPUSHA("0x%p", arena)
    for (u8 i = 0 ; i < TLARRSIZE(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload == NULL) break;

        arena->page[i].index = 0;
        tl_memory_set(arena->page[i].payload, 0, arena->page_size);
    }
    TLSTACKPOP
}

void tl_memory_arena_destroy(TLMemoryArena* arena) {
    TLSTACKPUSHA("0x%p", arena)
    const u8 index = tl_memory_arena_get_index(arena);
    tl_memory_arena_do_destroy(index);
    TLSTACKPOP
}

void *tl_memory_alloc(TLMemoryArena* arena, const u64 size, const TLMemoryTag tag) {
    TLSTACKPUSHA("0x%p, %llu, %d", arena, size, tag)
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired size
    // -------------------------------------------------
    if (size == 0) {
        TLFATAL("TLMemoryArena 0x%p allocation size must be greater then 0", arena)
        TLSTACKPOPV(NULL)
    }

    if (size > arena->page_size) {
        TLFATAL("TLMemoryArena with page size of %d bytes. It cannot allocate %d bytes", arena, arena->page_size, size)
        TLSTACKPOPV(NULL)
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {

        // Initialize a new TLMemoryPage
        if (arena->page[i].payload == NULL) {
            TLVERBOSE("TLMemoryArena 0x%p initializing page %d", arena, i)
            arena->page[i].payload = TLMALLOC(arena->page_size);
            TLMEMSET(arena->page[i].payload, 0, arena->page_size);

            found = i;
            break;
        }

        // check if the page support the desired size
        if (arena->page[i].index + size <= arena->page_size) {
            found = i;
            break;
        }
    }
    
    if (found == U8_MAX) {
        TLWARN("TLMemoryArena 0x%p no suitable TLMemoryPage", arena)
        TLSTACKPOPV(NULL)
    }
    // -------------------------------------------------
    // Adjust the TLMemoryArena internal state
    // -------------------------------------------------
    arena->allocated += size;
    arena->tagged_count[tag] += 1;
    arena->tagged_size[tag] += size;
    // -------------------------------------------------
    // Adjust the TLMemoryPage internal state
    // -------------------------------------------------
    void* address = arena->page[found].payload + arena->page[found].index;
    TLVERBOSE("TLMemoryArena 0x%p page %d [remaning %llu] :: allocating %llu ", arena, found, arena->page_size - arena->page[found].index, size)
    arena->page[found].index += size;
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    TLSTACKPOPV(address)
}

void tl_memory_set(void *block, const i32 value, const u64 size) {
    TLSTACKPUSHA("0x%p, %d, %llu", block, value, size)
    TLMEMSET(block, value, size);
    TLSTACKPOP
}

void tl_memory_copy(void *target, void *source, const u64 size) {
    TLSTACKPUSHA("0x%p, 0x%p, %llu", target, source, size)
    TLMEMCPY(target, source, size);
    TLSTACKPOP
}
// ########################################################
//                  LIFECYCLE FUNCTIONS
// ########################################################
#include "teleios/core/platform.h"

b8 tl_platform_initialize(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_platform_initialize");

    TLDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)
    TLTRACE("Initializing GLFW");
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        TLSTACKPOPV(FALSE)
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
#ifdef TLPLATFORM_APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Disable window framebuffer bits we don't need, because we render into offscreen FBO and blit to window.

    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_ALPHA_BITS, 0);

    TLDEBUG(
        "Window (%u x %u) :: %s",
        global->window.size.x,
        global->window.size.y,
        tl_string(global->window.title)
    )

    TLTRACE("GLFW creating window");
    global->window.handle = glfwCreateWindow(
        global->window.size.x,
        global->window.size.y,
        tl_string(global->window.title),
        NULL, NULL
    );
    
    if (global->window.handle == NULL) {
        TLERROR("Failed to create GLFW window");
        TLSTACKPOPV(FALSE)
    }

    global->window.visible = FALSE;
    global->window.focused = glfwGetWindowAttrib(global->window.handle, GLFW_FOCUSED) == GLFW_TRUE;
    global->window.maximized = glfwGetWindowAttrib(global->window.handle, GLFW_MAXIMIZED) == GLFW_TRUE;
    global->window.minimized = glfwGetWindowAttrib(global->window.handle, GLFW_ICONIFIED) == GLFW_TRUE;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (mode != NULL) {
        global->window.position.x = (mode->width - global->window.size.x) / 2;
        global->window.position.y = (mode->height - global->window.size.y) / 2;
    }

    glfwSetWindowPos(global->window.handle, global->window.position.x, global->window.position.y);

    TLTRACE("GLFW creating window callbacks");
    TLEvent event = {0};
    tl_event_submit(TL_EVENT_WINDOW_CREATED, &event);

    if (!tl_graphics_initialize()) {
        TLSTACKPOPV(FALSE)
    }

    TLDEBUG("Platform initialized in %llu micros", tl_profiler_time("tl_platform_initialize"));
    tl_profiler_end("tl_platform_initialize");
    TLSTACKPOPV(TRUE)
}

b8 tl_platform_terminate(void) {
    TLSTACKPUSH
    if (!tl_graphics_terminate()) {
        TLERROR("Failed to terminate graphics");
        TLSTACKPOPV(FALSE)
    }

    TLVERBOSE("Terminating GLFW");
    glfwTerminate();

    for (u8 i = 2 ; i < U8_MAX ; ++i) {
        if (global->memory.arenas[i] == NULL) continue;

        TLWARN("Removing dangling TLMemoryArena 0x%p", global->memory.arenas[i])
        tl_memory_arena_do_destroy(0);  // global->arenas.permanent
        tl_memory_arena_do_destroy(1);  // global->arenas.scene
        tl_memory_arena_do_destroy(2);  // global->arenas.frame
        tl_memory_set(global->memory.arenas[i], 0, sizeof(TLMemoryArena));
    }

    TLSTACKPOPV(TRUE)
}