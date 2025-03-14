#include "teleios/core/logger.h"
#include "teleios/core/string.h"

// ########################################################
//                    MEMORY FUNCTIONS
// ########################################################
#include "teleios/core/memory.h"

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

static const char *tl_memory_name(TLMemoryTag tag) {
    switch (tag) {
        case TL_MEMORY_BLOCK: return "TL_MEMORY_BLOCK";
        case TL_MEMORY_CONTAINER_LIST: return "TL_MEMORY_CONTAINER_LIST";
        case TL_MEMORY_CONTAINER_NODE: return "TL_MEMORY_CONTAINER_NODE";
        case TL_MEMORY_CONTAINER_ITERATOR: return "TL_MEMORY_CONTAINER_ITERATOR";
        case TL_MEMORY_STRING: return "TL_MEMORY_STRING";
        case TL_MEMORY_WINDOW: return "TL_MEMORY_WINDOW";
        case TL_MEMORY_MAXIMUM: return "TL_MEMORY_MAXIMUM";
    }

    return "???";
}

TLMemoryArena* tl_memory_arena_create(u64 size) {
    TLTRACE(">> tl_memory_arena_create(%d)", size)
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
        if (runtime->platform.memory.arenas[i] == NULL) {
            runtime->platform.memory.arenas[i] = arena;
            break;
        }
    }

    TLTRACE("Created TLMemoryArena 0x%p", arena)
    TLVERBOSE("TLMemoryArena 0x%p page size: %d bytes", arena, arena->page_size)
    
    TLTRACE("<< tl_memory_arena_create(%d)", size)
    return arena;
}

TLINLINE static b8 __tl_memory_arena_is_valid(TLMemoryArena* arena) {
    TLTRACE(">> __tl_memory_arena_is_valid(0x%p)", arena)

    if (arena == NULL) {
        TLWARN("TLMemoryArena is NULL")
        TLTRACE("<< __tl_memory_arena_is_valid(0x%p)", arena)
        return FALSE;
    }

    b8 found = FALSE;
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (runtime->platform.memory.arenas[i] == arena) {
            found = TRUE;
        }
    }

    if (!found) {
        TLWARN("Illegal TLMemoryArena 0x%p", arena)
    }

    TLTRACE("<< __tl_memory_arena_is_valid(0x%p)", arena)
    return found;
}


TLINLINE static void __tl_memory_arena_destroy(TLMemoryArena* arena) {
    TLTRACE(">> __tl_memory_arena_destroy(0x%p)", arena)
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arena->page[i].payload != NULL) {
            TLVERBOSE("Releasing page %d", i)
            TLFREE(arena->page[i].payload);
            arena->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
        if (arena->tagged_size[i] != 0) {
            TLVERBOSE("TLMemoryArena 0x%p at %-30s: [%03d] %llu bytes", arena, tl_memory_name(i), arena->tagged_count[i], arena->tagged_size[i]);
        }
    }

    TLFREE(arena);
    TLTRACE("<< __tl_memory_arena_destroy(0x%p)", arena)
    arena = NULL;
}

void tl_memory_arena_reset(TLMemoryArena* arena) {
    // TLTRACE(">> tl_memory_arena_reset(0x%p)", arena)
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arena->page[i].payload == NULL) break;

        arena->page[i].index = 0;
        TLVERBOSE("Zeroing memory page %u", i)
        tl_memory_set(arena->page[i].payload, 0, arena->page_size);
    }
    // TLTRACE("<< tl_memory_arena_reset(0x%p)", arena)
}

void tl_memory_arena_destroy(TLMemoryArena* arena) {
    TLTRACE(">> tl_memory_arena_destroy(0x%p)", arena)
    TLTRACE("Destroying TLMemoryArena 0x%p", arena)
    
    if (!__tl_memory_arena_is_valid(arena)) {
        TLTRACE("<< tl_memory_arena_destroy(0x%p)", arena)
        return;
    }

    __tl_memory_arena_destroy(arena);
    TLTRACE("<< tl_memory_arena_destroy(0x%p)", arena)
}

void *tl_memory_alloc(TLMemoryArena* arena, u64 size, TLMemoryTag tag) {
    TLTRACE(">> tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired size
    // -------------------------------------------------
    if (!__tl_memory_arena_is_valid(arena)) {
        TLTRACE("<< tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
        return NULL;
    }

    if (size == 0) {
        TLWARN("Allocation size must be greater then 0")
        TLTRACE("<< tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
        return NULL;
    }

    if (size > arena->page_size) {
        TLWARN("TLMemoryArena with page size of %d bytes. It cannot allocate %d bytes", arena->page_size, size)
        TLTRACE("<< tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
        return NULL;
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {
        // Initialize a new TLMemoryPage
        if (arena->page[i].payload == NULL) {
            TLVERBOSE("Initializing page %d", i)
            arena->page[i].payload = TLMALLOC(arena->page_size);
            
            found = i;
            break;
        }
        
        // Utilize the available TLMemoryPage
        if (arena->page[i].index + size <= arena->page_size) {
            TLVERBOSE("Using page %d", i)
            
            found = i;
            break;
        }
    }
    
    if (found == U8_MAX) {
        TLWARN("No suitable TLMemoryPage within the arena")
        return NULL;
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
    arena->page[found].index += size;
    TLVERBOSE("Page %d has %d bytes available", found, (arena->page_size - arena->page[found].index))
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    TLTRACE("<< tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
    return address;
}

void tl_memory_set(void *block, i32 value, u64 size) {
    TLTRACE(">> tl_memory_set(0x%p, %d, %llu)", block, value, size);
    TLMEMSET(block, value, size);
    TLTRACE("<< tl_memory_set(0x%p, %d, %llu)", block, value, size);
}

void tl_memory_copy(void *target, void *source, u64 size) {
    TLTRACE(">> tl_memory_copy(0x%p, 0x%p, %d)", target, source, size);
    TLMEMCPY(target, source, size);
    TLTRACE("<< tl_memory_copy(0x%p, 0x%p, %d)", target, source, size);
}
// ########################################################
//                  LIFECYCLE FUNCTIONS
// ########################################################
#include "teleios/core/platform.h"
#include "teleios/core/graphics.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

b8 tl_platform_initialize(void) {
    TLTRACE(">> tl_platform_initialize(void)")

    TLVERBOSE("Initializing GLFW");
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        return FALSE;
    }
    
    TLVERBOSE("Creating window");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef TLPLATFORM_APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    TLDEBUG("Window size (%u x %u)", runtime->platform.window.width, runtime->platform.window.height)
    TLDEBUG("Window title: %s", tl_string_text(runtime->platform.window.title))
    runtime->platform.window.handle = glfwCreateWindow(
        runtime->platform.window.width, 
        runtime->platform.window.height, 
        tl_string_text(runtime->platform.window.title), 
        NULL, NULL
    );
    
    if (runtime->platform.window.handle == NULL) {
        TLERROR("Failed to create GLFW window");
        return FALSE;       
    }

    if (!tl_graphics_initialize()) {
        return FALSE;
    }

    TLTRACE("<< tl_platform_initialize(void)")
    return TRUE;
}

b8 tl_platform_terminate(void) {
    TLTRACE(">> tl_platform_terminate(void)")
    if (!tl_graphics_terminate()) {
        TLERROR("Failed to terminate graphics");
    }

    TLVERBOSE("Terminating GLFW");
    glfwTerminate();

    __tl_memory_arena_destroy(runtime->arena_frame);
    __tl_memory_arena_destroy(runtime->arena_persistent);

    for (u8 i = 2 ; i < U8_MAX ; ++i) {
        TLMemoryArena* arena = runtime->platform.memory.arenas[i];
        if (arena == NULL) continue;

        TLWARN("Removing dangling TLMemoryArena 0x%p", arena)
        __tl_memory_arena_destroy(arena);
        tl_memory_set(runtime->platform.memory.arenas[i], 0, sizeof(TLMemoryArena));
    }

    tl_memory_set(&runtime->platform, 0, sizeof(runtime->platform));
    TLTRACE("<< tl_platform_terminate(void)")
    return TRUE;
}