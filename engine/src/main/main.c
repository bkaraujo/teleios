#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/application.h"

TLCore *core;

static TLEventStatus event_echo(TLEvent *event) {
    TLINFO("Processando evento 0x%p", event)
    return TL_EVENT_NOT_CONSUMED;
}

int main (const int argc, const char *argv[]) {
    if (argc != 2) {
        TLFATAL("argc != 2")
    }

    core = TLMALLOC(sizeof(TLCore));
    TLSTACKPUSHA("%i, 0%xp", argc, argv)

    core->arenas.permanent = tl_memory_arena_create(TLMEBIBYTES(32));
    tl_serializer_read(argv[1]);
    core->arenas.scene = tl_memory_arena_create(TLMEBIBYTES(32));
    core->arenas.frame = tl_memory_arena_create(TLMEBIBYTES(10));
    core->engine.ecs.entities = tl_list_create(core->arenas.permanent);
    core->engine.ecs.components = tl_list_create(core->arenas.permanent);

    tl_event_subscribe(TL_EVENT_WINDOW_CREATED, event_echo);

    if (!tl_platform_initialize()) {
        TLERROR("Platform failed to initialize");
        if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
        exit(99);
    }

    if (!tl_application_initialize()) {
        TLERROR("Application failed to initialize");
        if (!tl_application_terminate()) { TLERROR("Application failed to terminate"); }
        if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
        exit(99);
    }

    if (!tl_application_run()) { TLERROR("Application failed to execute"); }

    if (!tl_application_terminate()) { TLERROR("Application failed to terminate"); }

    tl_memory_arena_destroy(core->arenas.frame);
    tl_memory_arena_destroy(core->arenas.permanent);

    if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }

#if ! defined(TELEIOS_BUILD_RELEASE)
    TLDEBUG("core->stack used: %u", core->stack_maximum);
    TLDEBUG("core->stack reserved: %u", sizeof(core->stack) / sizeof(TLStackFrame));
#endif

    TLSTACKPOPV(0)
}
