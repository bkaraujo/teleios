#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/application/loader.h"
#include "teleios/globals.h"
#include "teleios/application.h"
    
TLGlobal *global;

int main (const int argc, const char *argv[]) {
    if (argc != 2) {
        TLERROR("argc != 2")
        TLSTACKPOPV(99)
    }

    TLINFO("Initializing %s", argv[1]);

    TLGlobal local = { 0 };
    global = &local;
    TLSTACKPUSHA("%i, 0%xp", argc, argv)

    global->platform.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->yaml = tl_string_clone(global->platform.arena, argv[1]);


    if (!tl_platform_initialize()) {
        TLERROR("Platform failed to initialize")
        if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_runtime_initialize()) {
        TLERROR("Runtime failed to initialize")
        if (!tl_runtime_terminate ()) TLERROR("Runtime failed to terminate")
        if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_application_load()) {
        TLERROR("Application failed to initialize");
        if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_application_initialize()) {
        TLERROR("Engine failed to initialize");
        if (!tl_application_terminate   ()) TLERROR("Application failed to terminate")
        if (!tl_runtime_terminate       ()) TLERROR("Runtime failed to terminate")
        if (!tl_platform_terminate      ()) TLFATAL("Platform failed to terminate")
        exit(99);
    }
    if (!tl_application_run         ()) TLERROR("Application failed to execute")
    if (!tl_application_terminate   ()) TLERROR("Application failed to terminate")
    if (!tl_runtime_terminate       ()) TLERROR("Runtime failed to terminate")

    tl_memory_arena_destroy(global->platform.arena);

    if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")

#if ! defined(TELEIOS_BUILD_RELEASE)
    TLDEBUG("global->stack used: %u", global->stack_maximum);
    TLDEBUG("global->stack reserved: %u", TLARRSIZE(global->stack, TLStackFrame));
#endif

    TLINFO("Exiting")
    TLSTACKPOPV(0)
}
