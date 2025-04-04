#include "teleios/core.h"
#include "teleios/runtime.h"

#include <asm-generic/errno-base.h>

#include "teleios/globals.h"
// #####################################################################################################################
//
//                                                     MEMORY
//
// #####################################################################################################################
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

static const char* tl_memory_name(const TLMemoryTag tag) {
    TL_STACK_PUSHA("%d", tag)
    switch (tag) {
        case TL_MEMORY_BLOCK                : TL_STACK_POPV("TL_MEMORY_BLOCK")
        case TL_MEMORY_SERIALIZER           : TL_STACK_POPV("TL_MEMORY_SERIALIZER")
        case TL_MEMORY_CONTAINER_LIST       : TL_STACK_POPV("TL_MEMORY_CONTAINER_LIST")
        case TL_MEMORY_CONTAINER_STACK      : TL_STACK_POPV("TL_MEMORY_CONTAINER_STACK")
        case TL_MEMORY_CONTAINER_NODE       : TL_STACK_POPV("TL_MEMORY_CONTAINER_NODE")
        case TL_MEMORY_CONTAINER_MAP        : TL_STACK_POPV("TL_MEMORY_CONTAINER_MAP")
        case TL_MEMORY_CONTAINER_MAP_ENTRY  : TL_STACK_POPV("TL_MEMORY_CONTAINER_MAP_ENTRY")
        case TL_MEMORY_CONTAINER_ITERATOR   : TL_STACK_POPV("TL_MEMORY_CONTAINER_ITERATOR")
        case TL_MEMORY_STRING               : TL_STACK_POPV("TL_MEMORY_STRING")
        case TL_MEMORY_PROFILER             : TL_STACK_POPV("TL_MEMORY_PROFILER")
        case TL_MEMORY_SCENE                : TL_STACK_POPV("TL_MEMORY_SCENE")
        case TL_MEMORY_ECS_COMPONENT        : TL_STACK_POPV("TL_MEMORY_ECS_COMPONENT")
        case TL_MEMORY_ULID                 : TL_STACK_POPV("TL_MEMORY_ULID")
        case TL_MEMORY_THREAD               : TL_STACK_POPV("TL_MEMORY_THREAD")
        case TL_MEMORY_MAXIMUM              : TL_STACK_POPV("TL_MEMORY_MAXIMUM")
    }

    TL_STACK_POPV("TL_MEMORY_???")
}

TLMemoryArena* tl_memory_arena_create(const u64 size) {
    TL_STACK_PUSHA("%d", size)
    // ----------------------------------------------------------
    // Create the memory arena
    // ----------------------------------------------------------
    TLMemoryArena *arena = tl_platform_memory_alloc(sizeof(TLMemoryArena));
    if (arena == NULL) TLFATAL("Failed to allocate TLMemoryArena");
    tl_platform_memory_set(arena, 0, sizeof(TLMemoryArena));
    arena->page_size = size;
    // ----------------------------------------------------------
    // Keep track of the created arena
    // ----------------------------------------------------------
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (global->platform.memory.arenas[i] == NULL) {
            global->platform.memory.arenas[i] = arena;
            TLTRACE("TLMemoryArena 0x%p created with page size of %d bytes", arena, arena->page_size)
            TL_STACK_POPV(arena)
        }
    }

    TLFATAL("Failed to allocate TLMemoryArena");
}

TL_INLINE static u8 tl_memory_arena_get_index(const TLMemoryArena *arena) {
    TL_STACK_PUSHA("0x%p", arena)

    if (arena == NULL) {
        TLWARN("TLMemoryArena is NULL")
        TL_STACK_POPV(false)
    }

    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (global->platform.memory.arenas[i] == NULL) continue;
        if (global->platform.memory.arenas[i] == arena) {
            TL_STACK_POPV(i)
        }
    }

    TLFATAL("TLMemoryArena 0x%p not found", arena)
}

TL_INLINE static void tl_memory_arena_do_destroy(const u8 index) {
    TL_STACK_PUSHA("%d", index)
    TLMemoryArena *arena = global->platform.memory.arenas[index];
    for (u32 i = 0 ; i < TL_ARR_LENGTH(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload != NULL) {
            TLVERBOSE("TLMemoryArena 0x%p releasing page %d", arena, i)
            tl_platform_memory_free(arena->page[i].payload);
            arena->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
        if (global->platform.memory.arenas[index]->tagged_size[i] != 0) {
            TLVERBOSE("TLMemoryArena 0x%p at %-30s: [%03d] %llu bytes", arena, tl_memory_name(i), arena->tagged_count[i], arena->tagged_size[i]);
        }
    }

    tl_platform_memory_free(arena);
    global->platform.memory.arenas[index] = NULL;

    TL_STACK_POP
}

void tl_memory_arena_reset(TLMemoryArena *arena) {
    TL_STACK_PUSHA("0x%p", arena)
    for (u32 i = 0 ; i < TL_ARR_LENGTH(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload == NULL) break;

        arena->page[i].index = 0;
        tl_memory_set(arena->page[i].payload, 0, arena->page_size);
    }
    TL_STACK_POP
}

void tl_memory_arena_destroy(TLMemoryArena *arena) {
    TL_STACK_PUSHA("0x%p", arena)
    const u8 index = tl_memory_arena_get_index(arena);
    tl_memory_arena_do_destroy(index);
    TL_STACK_POP
}

void *tl_memory_alloc(TLMemoryArena *arena, const u64 size, const TLMemoryTag tag) {
    TL_STACK_PUSHA("0x%p, %llu, %d", arena, size, tag)
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired size
    // -------------------------------------------------
    if (size == 0) {
        TLFATAL("TLMemoryArena 0x%p allocation size must be greater then 0", arena)
        TL_STACK_POPV(NULL)
    }

    if (size > arena->page_size) {
        TLFATAL("TLMemoryArena with page size of %d bytes. It cannot allocate %d bytes", arena, arena->page_size, size)
        TL_STACK_POPV(NULL)
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {

        // Initialize a new TLMemoryPage
        if (arena->page[i].payload == NULL) {
            TLTRACE("TLMemoryArena 0x%p initializing page %d", arena, i)
            arena->page[i].payload = tl_platform_memory_alloc(arena->page_size);
            tl_platform_memory_set(arena->page[i].payload, 0, arena->page_size);

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
        TL_STACK_POPV(NULL)
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
    TL_STACK_POPV(address)
}

void tl_memory_set(void *block, const i32 value, const u64 size) {
    TL_STACK_PUSHA("0x%p, %d, %llu", block, value, size)
    tl_platform_memory_set(block, value, size);
    TL_STACK_POP
}

void tl_memory_copy(void *target, void *source, const u64 size) {
    TL_STACK_PUSHA("0x%p, 0x%p, %llu", target, source, size)
    tl_platform_memory_copy(target, source, size);
    TL_STACK_POP
}
// #####################################################################################################################
//
//                                                     CONTAINER
//
// #####################################################################################################################

// #####################################################################################################################
//
//                                                     INPUT
//
// #####################################################################################################################
void tl_input_update() {
    tl_memory_copy(
        &global->application.frame.last.input,
        &global->application.frame.current.input,
        sizeof(global->platform.input)
    );

    tl_memory_copy(
        &global->application.frame.current.input,
        &global->platform.input,
        sizeof(global->platform.input)
    );
}

b8 tl_input_is_key_active(const i32 key) {
    TL_STACK_PUSHA("%d", key)
    const b8 is_active = global->application.frame.current.input.keyboard.key[key];
    TL_STACK_POPV(is_active)
}

b8 tl_input_is_key_pressed(const i32 key) {
    TL_STACK_PUSHA("%d", key)
    const b8 is_active = global->application.frame.current.input.keyboard.key[key];
    const b8 were_active = global->application.frame.last.input.keyboard.key[key];
    TL_STACK_POPV(!were_active & is_active)
}

b8 tl_input_is_key_released(const i32 key) {
    TL_STACK_PUSHA("%d", key)
    const b8 is_active = global->application.frame.current.input.keyboard.key[key];
    const b8 were_active = global->application.frame.last.input.keyboard.key[key];
    TL_STACK_POPV(were_active & !is_active)
}
// #####################################################################################################################
//
//                                                     FILESYSTEM
//
// #####################################################################################################################
TLString * tl_filesystem_get_parent(TLString *path) {
    TL_STACK_PUSHA("0x%p", path)
    if (path == NULL) TL_STACK_POPV(NULL)

    const u32 index = tl_string_last_index_of(path, '/');
    if (index == U32_MAX) TL_STACK_POPV(NULL)


    TL_CREATE_CHAR(value, index + 1)
    tl_char_copy(value, tl_string(path), index);
    TLString *parent = tl_string_clone(global->platform.arena, value);

    TL_STACK_POPV(parent)
}

// #####################################################################################################################
//
//                                                     SCRIPT
//
// #####################################################################################################################
#define TLSCRIPTERR(s) TL_STACK_POPV(luaL_error(state, s))

static i32 tl_script__application_exit(lua_State *state) {
    TL_STACK_PUSHA("0x%p", state)
    // =========================================================================
    // Parameters validation
    // =========================================================================
    if (lua_gettop(state) != 0) TLSCRIPTERR("No parameter were expected.")
    // =========================================================================
    // Request execution
    // =========================================================================
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
    // =========================================================================
    // LUA stack push
    // =========================================================================
    TL_STACK_POPV(0)
}

static i32 tl_script__is_key_pressed(lua_State *state) {
    TL_STACK_PUSHA("0x%p", state)
    // =========================================================================
    // Parameters validation
    // =========================================================================
    if (lua_gettop(state) != 1) TLSCRIPTERR("Expected a single value: isKeyPressed(KEY)")
    if (!lua_isinteger(state, 1)) TLSCRIPTERR("parameter [KEY] must be a valid key")
    // =========================================================================
    // Request execution
    // =========================================================================
    const i32 key = lua_tonumber(state, 1);
    lua_pushboolean(state, tl_input_is_key_pressed(key));
    // =========================================================================
    // LUA stack push
    // =========================================================================
    TL_STACK_POPV(1)
}

static i32 tl_script__is_key_released(lua_State *state) {
    TL_STACK_PUSHA("0x%p", state)
    // =========================================================================
    // Parameters validation
    // =========================================================================
    if (lua_gettop(state) != 1) TLSCRIPTERR("Expected a single value: isKeyReleased(KEY)")
    if (!lua_isinteger(state, 1)) TLSCRIPTERR("parameter [KEY] must be a valid key")
    // =========================================================================
    // Request execution
    // =========================================================================
    const i32 key = lua_tonumber(state, 1);
    lua_pushboolean(state, tl_input_is_key_released(key));
    // =========================================================================
    // LUA stack push
    // =========================================================================
    TL_STACK_POPV(1)
}

b8 tl_script_initialize(void) {
    TL_STACK_PUSH

    TLTRACE("Initializing Script Engine");

    TLDEBUG("LUA_VERSION %s.%s.%s", LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    global->platform.script.state = luaL_newstate();
    if (global->platform.script.state == NULL) {
        TLERROR("Failed to initialize LUA")
    }

    luaL_openlibs(global->platform.script.state);

    const luaL_Reg teleios_input[] = {
        {"isKeyPressed", tl_script__is_key_pressed},
        {"isKeyReleased", tl_script__is_key_released},
        {NULL, NULL}
    };

    luaL_newlib(global->platform.script.state, teleios_input);
    lua_setglobal(global->platform.script.state, "teleios_input");

    const luaL_Reg teleios_application[] = {
        {"exit", tl_script__application_exit},
        {NULL, NULL}
    };

    luaL_newlib(global->platform.script.state, teleios_application);
    lua_setglobal(global->platform.script.state, "teleios_application");

    TL_STACK_POPV(true)
}

b8 tl_script_terminate(void) {
    TL_STACK_PUSH
    TLTRACE("Terminating Script Engine");
    lua_close(global->platform.script.state);
    global->platform.script.state = NULL;
    TL_STACK_POPV(true)
}
// #####################################################################################################################
//
//                                                     ULID
//
// #####################################################################################################################
#ifdef TLPLATFORM_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   pragma comment(lib, "advapi32.lib")
#elif TLPLATFORM_LINUX
// #   define _GNU_SOURCE
#   include <sys/time.h>
#   include <sys/syscall.h>
#elif TLPLATFORM_POSIX
#   define _POSIX_C_SOURCE 200112L
#   include <sys/time.h>
#else
#   error "!!!"
#endif

#include <time.h>

static int platform_entropy(void *buf, const i32 len) {
    TL_STACK_PUSHA("0x%p, %d", buf, len)
#if _WIN32
    BOOLEAN NTAPI SystemFunction036(PVOID, ULONG);
    TL_STACK_POPV(!SystemFunction036(buf, len))
#elif __linux__
    i32 result = syscall(SYS_getrandom, buf, len, 0);
    TL_STACK_POPV(result != len)
#else
    int r = 0;
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) {
        r = fread(buf, len, 1, f);
        fclose(f);
    }
    TL_STACK_POPV(!r)
#endif
}

TLUlidGenerator* tl_ulid_generator_init(TLMemoryArena *arena, const i32 flags) {
    TL_STACK_PUSHA("0x%p, %d", arena, flags)
    TLUlidGenerator* generator = tl_memory_alloc(arena, sizeof(TLUlidGenerator), TL_MEMORY_ULID);

    generator->timestamp = 0;
    generator->flags = flags;
    generator->i = generator->j = 0;
    for (i32 i = 0; i < 256; i++)
        generator->s[i] = i;

    /* RC4 is used to fill the random segment of ULIDs. It's tiny,
     * simple, perfectly sufficient for the task (assuming it's seeded
     * properly), and doesn't require fixed-width integers. It's not the
     * fastest option, but it's plenty fast for the task.
     *
     * Besides, when we're in a serious hurry in normal operation (not
     * in "relaxed" mode), we're incrementing the random field much more
     * often than generating fresh random bytes.
     */

    u8 key[256] = {0};
    if (!platform_entropy(key, 256)) {
        /* Mix entropy into the RC4 state. */
        for (i32 i = 0, j = 0; i < 256; ++i) {
            j = (j + generator->s[i] + key[i]) & 0xff;

            const i32 tmp = generator->s[i];
            generator->s[i] = generator->s[j];
            generator->s[j] = tmp;
        }

        TL_STACK_POPV(generator)
    }

    if (!(flags & ULID_SECURE)) {
        /* Failed to read entropy from OS, so generate some. */
        u64 now, n = 0;
        const u64 start = tl_time_epoch_millis();

        do {
            struct {
                clock_t clk;
                u64 ts;
                i64 n;
                void *stackgap;
            } noise;

            noise.ts = now = tl_time_epoch_millis();
            noise.clk = clock();
            noise.stackgap = &noise;
            noise.n = (i64) n;

            const u8 *k = (u8 *)&noise;
            for (i32 i = 0, j = 0; i < 256; ++i) {
                j = (j + generator->s[i] + k[i % sizeof(noise)]) & 0xff;
                const i32 tmp = generator->s[i];
                generator->s[i] = generator->s[j];
                generator->s[j] = tmp;
            }
        } while (n++ < 1UL << 16 || now - start < 500000ULL);
        TL_STACK_POPV(generator)
    }

    TL_STACK_POPV(NULL)
}

void tl_ulid_encode(TLUlid* ulid, const u8 binary[16]) {
    static const i8 set[256] = {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a
    };

    ulid->text[ 0] = set[ binary[ 0] >> 5];
    ulid->text[ 1] = set[ binary[ 0] >> 0];
    ulid->text[ 2] = set[ binary[ 1] >> 3];
    ulid->text[ 3] = set[(binary[ 1] << 2 | binary[ 2] >> 6) & 0x1f];
    ulid->text[ 4] = set[ binary[ 2] >> 1];
    ulid->text[ 5] = set[(binary[ 2] << 4 | binary[ 3] >> 4) & 0x1f];
    ulid->text[ 6] = set[(binary[ 3] << 1 | binary[ 4] >> 7) & 0x1f];
    ulid->text[ 7] = set[ binary[ 4] >> 2];
    ulid->text[ 8] = set[(binary[ 4] << 3 | binary[ 5] >> 5) & 0x1f];
    ulid->text[ 9] = set[ binary[ 5] >> 0];
    ulid->text[10] = set[ binary[ 6] >> 3];
    ulid->text[11] = set[(binary[ 6] << 2 | binary[ 7] >> 6) & 0x1f];
    ulid->text[12] = set[ binary[ 7] >> 1];
    ulid->text[13] = set[(binary[ 7] << 4 | binary[ 8] >> 4) & 0x1f];
    ulid->text[14] = set[(binary[ 8] << 1 | binary[ 9] >> 7) & 0x1f];
    ulid->text[15] = set[ binary[ 9] >> 2];
    ulid->text[16] = set[(binary[ 9] << 3 | binary[10] >> 5) & 0x1f];
    ulid->text[17] = set[ binary[10] >> 0];
    ulid->text[18] = set[ binary[11] >> 3];
    ulid->text[19] = set[(binary[11] << 2 | binary[12] >> 6) & 0x1f];
    ulid->text[20] = set[ binary[12] >> 1];
    ulid->text[21] = set[(binary[12] << 4 | binary[13] >> 4) & 0x1f];
    ulid->text[22] = set[(binary[13] << 1 | binary[14] >> 7) & 0x1f];
    ulid->text[23] = set[ binary[14] >> 2];
    ulid->text[24] = set[(binary[14] << 3 | binary[15] >> 5) & 0x1f];
    ulid->text[25] = set[ binary[15] >> 0];
    ulid->text[26] = 0;
}

// static b8 ulid_decode(u8 ulid[16], const i8 *s) {
//     TL_STACK_PUSHA("0x%p, 0x%p", ulid, s)
//     static const i8 v[] = {
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//         0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//         0x08, 0x09,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
//         0x11, 0x01, 0x12, 0x13, 0x01, 0x14, 0x15, 0x00,
//         0x16, 0x17, 0x18, 0x19, 0x1a,   -1, 0x1b, 0x1c,
//         0x1d, 0x1e, 0x1f,   -1,   -1,   -1,   -1,   -1,
//           -1, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
//         0x11, 0x01, 0x12, 0x13, 0x01, 0x14, 0x15, 0x00,
//         0x16, 0x17, 0x18, 0x19, 0x1a,   -1, 0x1b, 0x1c,
//         0x1d, 0x1e, 0x1f,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1
//     };
//     if (v[(i32)s[0]] > 7)
//         TL_STACK_POPV(true)
//
//     for (i32 i = 0; i < 26; i++)
//         if (v[(i32)s[i]] == -1)
//             TL_STACK_POPV(true)
//
//     ulid[ 0] = v[(i32)s[ 0]] << 5 | v[(i32)s[ 1]] >> 0;
//     ulid[ 1] = v[(i32)s[ 2]] << 3 | v[(i32)s[ 3]] >> 2;
//     ulid[ 2] = v[(i32)s[ 3]] << 6 | v[(i32)s[ 4]] << 1 | v[(i32)s[ 5]] >> 4;
//     ulid[ 3] = v[(i32)s[ 5]] << 4 | v[(i32)s[ 6]] >> 1;
//     ulid[ 4] = v[(i32)s[ 6]] << 7 | v[(i32)s[ 7]] << 2 | v[(i32)s[ 8]] >> 3;
//     ulid[ 5] = v[(i32)s[ 8]] << 5 | v[(i32)s[ 9]] >> 0;
//     ulid[ 6] = v[(i32)s[10]] << 3 | v[(i32)s[11]] >> 2;
//     ulid[ 7] = v[(i32)s[11]] << 6 | v[(i32)s[12]] << 1 | v[(i32)s[13]] >> 4;
//     ulid[ 8] = v[(i32)s[13]] << 4 | v[(i32)s[14]] >> 1;
//     ulid[ 9] = v[(i32)s[14]] << 7 | v[(i32)s[15]] << 2 | v[(i32)s[16]] >> 3;
//     ulid[10] = v[(i32)s[16]] << 5 | v[(i32)s[17]] >> 0;
//     ulid[11] = v[(i32)s[18]] << 3 | v[(i32)s[19]] >> 2;
//     ulid[12] = v[(i32)s[19]] << 6 | v[(i32)s[20]] << 1 | v[(i32)s[21]] >> 4;
//     ulid[13] = v[(i32)s[21]] << 4 | v[(i32)s[22]] >> 1;
//     ulid[14] = v[(i32)s[22]] << 7 | v[(i32)s[23]] << 2 | v[(i32)s[24]] >> 3;
//     ulid[15] = v[(i32)s[24]] << 5 | v[(i32)s[25]] >> 0;
//
//     TL_STACK_POPV(false)
// }

TLUlid* tl_ulid_generate(TLMemoryArena *arena, TLUlidGenerator *generator) {
    TL_STACK_PUSHA("0x%p, 0x%p", arena, generator)
    const u64 timestamp = tl_time_epoch_micros() / 1000;
    TLUlid* ulid = tl_memory_alloc(arena, sizeof(TLUlid), TL_MEMORY_ULID);

    if (!(generator->flags & ULID_RELAXED) && generator->timestamp == timestamp) {
        /* Chance of 80-bit overflow is so small that it's not considered. */
        for (i32 i = 15; i > 5; i--)
            if (++generator->last[i])
                break;

        tl_ulid_encode(ulid, generator->last);
        TL_STACK_POPV(ulid)
    }

    /* Fill out timestamp */
    generator->timestamp = timestamp;
    generator->last[0] = timestamp >> 40;
    generator->last[1] = timestamp >> 32;
    generator->last[2] = timestamp >> 24;
    generator->last[3] = timestamp >> 16;
    generator->last[4] = timestamp >>  8;
    generator->last[5] = timestamp >>  0;

    /* Fill out random section */
    for (i32 k = 0; k < 10; k++) {
        generator->i = (generator->i + 1) & 0xff;
        generator->j = (generator->j + generator->s[generator->i]) & 0xff;

        const i32 tmp = generator->s[generator->i];
        generator->s[generator->i] = generator->s[generator->j];
        generator->s[generator->j] = tmp;
        generator->last[6 + k] = generator->s[(generator->s[generator->i] + generator->s[generator->j]) & 0xff];
    }

    if (generator->flags & ULID_PARANOID)
        generator->last[6] &= 0x7f;

    tl_ulid_encode(ulid, generator->last);
    TL_STACK_POPV(ulid)
}

TLString* tl_ulid(TLMemoryArena *arena, TLUlid * ulid) {
    TL_STACK_PUSHA("0x%p, 0x%p", arena, ulid)
    TLString* string = tl_string_clone(arena, ulid->text);
    TL_STACK_POPV(string)
}
// #####################################################################################################################
//
//                                             WINDOWING
//
// #####################################################################################################################
static void tl_window_callback_window_closed(GLFWwindow* window) {
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
}

static void tl_window_callback_window_pos(GLFWwindow* _, const int xPos, const int yPos) {
    global->platform.window.position.x = xPos;
    global->platform.window.position.y = yPos;

    TLEvent event = { 0 };
    event.i32[0] = global->platform.window.position.x;
    event.i32[1] = global->platform.window.position.y;

    tl_event_submit(TL_EVENT_WINDOW_MOVED, &event);
}

static void tl_window_callback_window_size(GLFWwindow* window, const int width, const int height) {
    global->platform.window.size.x = width;
    global->platform.window.size.y = height;

    TLEvent event = { 0 };
    event.i32[0] = global->platform.window.size.x;
    event.i32[1] = global->platform.window.size.y;

    tl_event_submit(TL_EVENT_WINDOW_RESIZED, &event);
}

static void tl_window_callback_window_focus(GLFWwindow* window, const i32 focused) {
    global->platform.window.focused = focused;

    tl_event_submit(focused ? TL_EVENT_WINDOW_FOCUS_GAINED : TL_EVENT_WINDOW_FOCUS_LOST, NULL);
}

static void tl_window_callback_window_minimized(GLFWwindow* window, const i32 minimized) {
    global->platform.window.maximized = false;
    global->platform.window.minimized = minimized;

    tl_event_submit(minimized ? TL_EVENT_WINDOW_MINIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_window_maximize(GLFWwindow* window, const i32 maximized) {
    global->platform.window.maximized = maximized;
    global->platform.window.minimized = false;

    tl_event_submit(maximized ? TL_EVENT_WINDOW_MAXIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_input_keyboard(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
    if (action == GLFW_REPEAT) return;

    i32 type;
    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_KEY_PRESSED;
        global->platform.input.keyboard.key[key] = true;
    } else {
        type = TL_EVENT_INPUT_KEY_RELEASED;
        global->platform.input.keyboard.key[key] = false;
    }

    TLEvent event = { 0 };
    event.u32[0] = key;
    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_position(GLFWwindow* window, const double xpos, const double ypos) {
    global->platform.input.cursor.position_x = (u32) xpos;
    global->platform.input.cursor.position_y = (u32) ypos;

    TLEvent event = { 0 };
    event.u32[0] = global->platform.input.cursor.position_x;
    event.u32[1] = global->platform.input.cursor.position_y;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_MOVED, &event);
}

static void tl_window_callback_input_cursor_button(GLFWwindow* window, const int button, const int action, const int mods) {
    i32 type;

    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_CURSOR_PRESSED;
        global->platform.input.cursor.button[button] = true;
    } else {
        type = TL_EVENT_INPUT_CURSOR_RELEASED;
        global->platform.input.cursor.button[button] = false;
    }

    global->platform.input.cursor.button[button] = action == GLFW_PRESS;

    TLEvent event = { 0 };
    event.u32[0] = button;

    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_scroll(GLFWwindow* window, const double xoffset, const double yoffset) {
    global->platform.input.cursor.scroll_x = xoffset > 0 ? 1 : xoffset < 0 ? -1 : 0;
    global->platform.input.cursor.scroll_y = yoffset > 0 ? 1 : yoffset < 0 ? -1 : 0;

    TLEvent event = { 0 };
    event.u8[0] = global->platform.input.cursor.scroll_x;
    event.u8[1] = global->platform.input.cursor.scroll_y;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_SCROLLED, &event);
}

static void tl_window_callback_input_cursor_entered(GLFWwindow* window, const int entered) {
    global->platform.input.cursor.hoover = entered == GLFW_TRUE;
    tl_event_submit(entered ? TL_EVENT_INPUT_CURSOR_ENTERED : TL_EVENT_INPUT_CURSOR_EXITED, NULL);
}

static b8 tl_window_create(void) {
    TL_STACK_PUSH

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
#ifdef TLPLATFORM_APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_true);
#endif

    // --------------------------------------------------------------------------------------
    // Disable window framebuffer bits we don't need, because we render into offscreen FBO and blit to window.
    // --------------------------------------------------------------------------------------
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_ALPHA_BITS, 0);
    // --------------------------------------------------------------------------------------
    // Create the platform surface
    // --------------------------------------------------------------------------------------
    global->platform.window.handle = glfwCreateWindow(
        global->platform.window.size.x,
        global->platform.window.size.y,
        tl_string(global->platform.window.title),
        NULL, NULL
    );

    if (global->platform.window.handle == NULL) {
        TLERROR("Failed to create GLFW window");
        TL_STACK_POPV(false)
    }
    // --------------------------------------------------------------------------------------
    // Cache state
    // --------------------------------------------------------------------------------------
    global->platform.window.visible = false;
    global->platform.window.focused = glfwGetWindowAttrib(global->platform.window.handle, GLFW_FOCUSED) == GLFW_TRUE;
    global->platform.window.maximized = glfwGetWindowAttrib(global->platform.window.handle, GLFW_MAXIMIZED) == GLFW_TRUE;
    global->platform.window.minimized = glfwGetWindowAttrib(global->platform.window.handle, GLFW_ICONIFIED) == GLFW_TRUE;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (mode != NULL) {
        global->platform.window.position.x = (mode->width - global->platform.window.size.x) / 2;
        global->platform.window.position.y = (mode->height - global->platform.window.size.y) / 2;
    }

    glfwSetWindowPos(global->platform.window.handle, global->platform.window.position.x, global->platform.window.position.y);
    // --------------------------------------------------------------------------------------
    // Window callbacks
    // --------------------------------------------------------------------------------------
    glfwSetWindowCloseCallback      (global->platform.window.handle, tl_window_callback_window_closed);
    glfwSetWindowPosCallback        (global->platform.window.handle, tl_window_callback_window_pos);
    glfwSetWindowSizeCallback       (global->platform.window.handle, tl_window_callback_window_size);
    glfwSetWindowFocusCallback      (global->platform.window.handle, tl_window_callback_window_focus);
    glfwSetWindowIconifyCallback    (global->platform.window.handle, tl_window_callback_window_minimized);
    glfwSetWindowMaximizeCallback   (global->platform.window.handle, tl_window_callback_window_maximize);
    // --------------------------------------------------------------------------------------
    // Input callbacks
    // --------------------------------------------------------------------------------------
    glfwSetKeyCallback              (global->platform.window.handle, tl_window_callback_input_keyboard);
    glfwSetMouseButtonCallback      (global->platform.window.handle, tl_window_callback_input_cursor_button);
    glfwSetCursorPosCallback        (global->platform.window.handle, tl_window_callback_input_cursor_position);
    glfwSetScrollCallback           (global->platform.window.handle, tl_window_callback_input_cursor_scroll);
    glfwSetCursorEnterCallback      (global->platform.window.handle, tl_window_callback_input_cursor_entered);

    TL_STACK_POPV(true)
}
// #####################################################################################################################
//
//                                                     THREAD
//
// #####################################################################################################################
#include <pthread.h>

typedef struct {
    pthread_t handle;
    PFN_task task;
    u64 created_at;
    u64 task_acquired_at;
    u64 id;
} TLThread;

typedef struct {
    TLThread thread[TL_THREAD_POOL_SIZE];
    PFN_task stack[U8_MAX];
    u8 index;

    pthread_mutex_t mutex;
    pthread_cond_t condition;
} TLThreadPool;

static TLThreadPool thread_pool;

static void* tl_thread_runner(void *parameter) {
    TLThread *thread = parameter;
    thread->id = pthread_self();
    TLTRACE("Initializing ThreadPoll Thread")

    while (global->application.running) {

        pthread_mutex_lock(&thread_pool.mutex);
        while (thread_pool.index == U8_MAX) {
            pthread_cond_wait(&thread_pool.condition, &thread_pool.mutex);
        }

        thread->task_acquired_at = tl_time_epoch_micros();
        thread->task = thread_pool.stack[thread_pool.index];
        TLTRACE("Accepting function 0x%p", thread->task)

        thread_pool.stack[thread_pool.index] = NULL;
        thread_pool.index--;

        pthread_mutex_unlock(&thread_pool.mutex);
        thread->task();
    }

    return NULL;
}

b8 tl_thread_initialize(void) {
    TL_STACK_PUSH
    TLTRACE("Initializing Threadpool");

    pthread_cond_init(&thread_pool.condition, NULL);
    pthread_mutex_init(&thread_pool.mutex, NULL);
    pthread_mutex_lock(&thread_pool.mutex);
    tl_memory_set(thread_pool.stack, 0, TL_ARR_SIZE(thread_pool.stack, u64*));

    for (u8 i = 0 ; i < TL_THREAD_POOL_SIZE ; ++i) {
        thread_pool.thread[i].created_at = tl_time_epoch_micros();
        if (pthread_create(&thread_pool.thread[i].handle, NULL, tl_thread_runner, &thread_pool.thread[i]) != 0) {
            TLERROR("Failed to create Threadpool thread");
            TL_STACK_POPV(false)
        }
    }

    TL_STACK_POPV(true)
}

b8 tl_thread_terminate(void) {
    TL_STACK_PUSH

    TLTRACE("Terminating Threadpool");

    for (u8 i = 0 ; i < TL_THREAD_POOL_SIZE ; ++i) {
        if (thread_pool.thread[i].handle != 0) {
            TLTRACE("Cancelling Threadpool thread %llu", thread_pool.thread[i].id);
            const i32 result = pthread_cancel(thread_pool.thread[i].handle);
            if (result != 0) {
                if (result == ESRCH) { TLERROR("Failed to cancel thread: no thread with the id %#x could be found.", thread_pool.thread[i].id); }
                else                 { TLERROR("Failed to cancel thread: an unknown error has occurred. errno=%i", result); }

                TL_STACK_POPV(false)
            }
        }
    }

    pthread_mutex_destroy(&thread_pool.mutex);
    pthread_cond_destroy(&thread_pool.condition);

    TL_STACK_POPV(true)
}

static u8 tl_thread_submit(PFN_task task) {
    TL_STACK_PUSHA("0x%p", task);

    pthread_mutex_lock(&thread_pool.mutex);
    if (thread_pool.index == U8_MAX - 1) {
        TLWARN("ThreadPool is full, rejecting task")
        TL_STACK_POPV(false)
    }

    thread_pool.stack[thread_pool.index] = task;
    thread_pool.index++;
    pthread_mutex_unlock(&thread_pool.mutex);

    pthread_cond_signal(&thread_pool.condition);

    TL_STACK_POPV(true)
}

void tl_thread_fire_and_forget(PFN_task task) {
    TL_STACK_PUSHA("0x%p", task);
    tl_thread_submit(task);
    TL_STACK_POP
}

void tl_thread_fire_and_wait(PFN_task task, const u64 timeout) {
    TL_STACK_PUSHA("0x%p, %llu", task, timeout);
    tl_thread_submit(task);
    TL_STACK_POP
}
// #####################################################################################################################
//
//                                                     LIFECYCLE
//
// #####################################################################################################################
b8 tl_runtime_initialize(void) {
    TL_STACK_PUSH

    if (global->properties == NULL || tl_map_length(global->properties) == 0) {
        TLERROR("Failed to read runtime properties")
        TL_STACK_POPV(false)
    }

    TLIterator *it = tl_map_keys(global->properties);
    for (TLString* key = tl_iterator_next(it); key != NULL; key = tl_iterator_next(it)) {
        if (tl_string_start_with(key, "application.scenes.")) continue;
        const char *value = tl_string(tl_map_get(global->properties, tl_string(key)));

        if (tl_string_equals(key, "application.version")) {
            global->application.version = tl_string_wrap(global->platform.arena, value);
            TLTRACE("global->application.version = %s", tl_string(global->application.version))
            continue;
        }

        if ( ! tl_string_start_with(key, "engine.")) continue;

        if (tl_string_equals(key, "engine.logging.level")) {
            if (tl_char_equals(value, "verbose")) { tl_logger_loglevel(TL_LOG_LEVEL_VERBOSE); continue; }
            if (tl_char_equals(value,   "trace")) { tl_logger_loglevel(  TL_LOG_LEVEL_TRACE); continue; }
            if (tl_char_equals(value,   "debug")) { tl_logger_loglevel(  TL_LOG_LEVEL_DEBUG); continue; }
            if (tl_char_equals(value,    "info")) { tl_logger_loglevel(   TL_LOG_LEVEL_INFO); continue; }
            if (tl_char_equals(value,    "warn")) { tl_logger_loglevel(   TL_LOG_LEVEL_WARN); continue; }
            if (tl_char_equals(value,   "error")) { tl_logger_loglevel(  TL_LOG_LEVEL_ERROR); continue; }
            if (tl_char_equals(value,   "fatal")) { tl_logger_loglevel(  TL_LOG_LEVEL_FATAL); continue; }
        }


        if (tl_string_start_with(key, "engine.graphics.")) {
            if (tl_string_equals(key, "engine.graphics.vsync")) {
                global->platform.graphics.vsync = tl_char_equals(value, "true");
                TLTRACE("global->platform.graphics.vsync = %d", global->platform.graphics.vsync)
                continue;
            }

            if (tl_string_equals(key, "engine.graphics.wireframe")) {
                global->platform.graphics.wireframe = tl_char_equals(value, "true");
                TLTRACE("global->platform.graphics.wireframe = %d", global->platform.graphics.wireframe)
                continue;
            }
        }

        if (tl_string_start_with(key, "engine.simulation.")) {
            if (tl_string_equals(key, "engine.simulation.step")) {
                u8 step = strtol(value, (void*)(value + tl_char_length(value)), 10);
                if (step == 0) {
                    TLWARN("Failed to read [%s] assuming 24", tl_string(key));
                    step = 24;
                }

                global->application.simulation.step = 1.0f / (f64) step;
                TLTRACE("global->simulation.step = %f", global->application.simulation.step)
                continue;
            }
        }

        if (tl_string_start_with(key, "engine.window.")) {
            if (tl_string_equals(key, "engine.window.title")) {
                global->platform.window.title = tl_string_clone(global->platform.arena, value);
                TLTRACE("global->platform.window.title = %s", tl_string(global->platform.window.title))
                continue;
            }

            if (tl_string_equals(key, "engine.window.size")) {
                if (tl_char_equals(value,  "SD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_SD; }
                if (tl_char_equals(value,  "HD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_HD; }
                if (tl_char_equals(value, "FHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_FHD; }
                if (tl_char_equals(value, "QHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_QHD; }
                if (tl_char_equals(value, "UHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_UHD; }

                global->platform.window.size.y = (global->platform.window.size.x * 9) / 16;
                TLTRACE("global->platform.window.size = %u x %u", global->platform.window.size.x, global->platform.window.size.y)
                continue;
            }
        }
    }

    if (!tl_thread_initialize()) {
        TLERROR("Failed to initialize threadpool")
        TL_STACK_POPV(false)
    }

    if (!tl_window_create()) {
        TLERROR("Failed to create application window");
        TL_STACK_POPV(false)
    }

    if (!tl_script_initialize()) {
        TLERROR("Failed to initialize script engine");
        TL_STACK_POPV(false)
    }

    if (!tl_graphics_initialize()) {
        TLERROR("Failed to initialize Graphics API")
        TL_STACK_POPV(false)
    }

    TL_STACK_POPV(true)
}

b8 tl_runtime_terminate(void) {
    TL_STACK_PUSH

    if (!tl_script_terminate()) {
        TLERROR("Failed to terminate script engine");
        TL_STACK_POPV(false)
    }

    if (!tl_graphics_terminate()) {
        TLERROR("Failed to terminate graphics engine");
        TL_STACK_POPV(false)
    }


    if (!tl_thread_terminate()) {
        TLERROR("Failed to terminate threadpool")
        TL_STACK_POPV(false)
    }

    TL_STACK_POPV(true)
}