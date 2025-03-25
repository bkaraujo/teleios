#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"
#include "teleios/application/application.h"

TLGlobal *global;

int main (const int argc, const char *argv[]) {
    if (argc != 2) {
        TLERROR("argc != 2")
        TLSTACKPOPV(99)
    }

    TLGlobal local = { 0 };
    global = &local;
    TLSTACKPUSHA("%i, 0%xp", argc, argv)

    global->platform.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->application.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->application.scene.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->application.frame.arena = tl_memory_arena_create(TLMEBIBYTES(10));


    global->yaml = tl_string_clone(global->application.arena, argv[1]);
    if (!tl_serializer_read_yaml()) {
        TLERROR("Invalid application yaml");
        TLSTACKPOPV(99)
    }

    if (!tl_platform_initialize()) {
        TLERROR("Platform failed to initialize")
        if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_application_initialize()) {
        TLERROR("Application failed to initialize");
        if (!tl_application_terminate()) TLERROR("Application failed to terminate")
        if (!tl_platform_terminate   ()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_engine_initialize()) {
        TLERROR("Engine failed to initialize");
        if (!tl_engine_terminate     ()) TLERROR("Engine failed to terminate")
        if (!tl_application_terminate()) TLERROR("Application failed to terminate")
        if (!tl_platform_terminate   ()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_engine_run           ()) TLERROR("Engine failed to execute")
    if (!tl_application_terminate()) TLERROR("Application failed to terminate")
    if (!tl_engine_terminate     ()) TLERROR("Engine failed to terminate")

    tl_memory_arena_destroy(global->application.frame.arena);
    tl_memory_arena_destroy(global->application.scene.arena);
    tl_memory_arena_destroy(global->application.arena);
    tl_memory_arena_destroy(global->platform.arena);

    if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")

#if ! defined(TELEIOS_BUILD_RELEASE)
    TLDEBUG("global->stack used: %u", global->stack_maximum);
    TLDEBUG("global->stack reserved: %u", TLARRSIZE(global->stack, TLStackFrame));
#endif

    TLSTACKPOPV(0)
}
