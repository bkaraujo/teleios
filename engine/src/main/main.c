#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"
#include "teleios/application.h"

TLGlobal *global;

static TLEventStatus event_echo(TLEvent *event) {
    TLINFO("Processando evento 0x%p", event)
    return TL_EVENT_NOT_CONSUMED;
}

int main (const int argc, const char *argv[]) {
    if (argc != 2) {
        TLFATAL("argc != 2")
    }

    TLGlobal local = { 0 };
    global = &local;
    TLSTACKPUSHA("%i, 0%xp", argc, argv)

    global->platform.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->application.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->application.scene.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->application.frame.arena = tl_memory_arena_create(TLMEBIBYTES(10));


    global->yaml = tl_string_clone(global->application.arena, argv[1]);
    tl_serializer_read();

    tl_event_subscribe(TL_EVENT_WINDOW_CREATED, event_echo);

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

    if (!tl_application_run      ()) TLERROR("Application failed to execute")
    if (!tl_application_terminate()) TLERROR("Application failed to terminate")

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
