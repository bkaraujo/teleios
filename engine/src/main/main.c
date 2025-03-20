#include "teleios/core.h"
#include "teleios/core/platform.h"
#include "teleios/application.h"
#include "teleios/core/serializer.h"

TLRuntime *runtime;

static TLEventStatus event_echo(TLEvent *event) {
    TLINFO("Processando evento 0x%p", event)
    return TL_EVENT_NOT_CONSUMED;
}

int main (const int argc, const char *argv[]) {
    if (argc != 2) {
        TLFATAL("argc != 2")
    }

    runtime = TLMALLOC(sizeof(TLRuntime));
    TLSTACKPUSHA("%i, 0%xp", argc, argv)

    runtime->arenas.permanent = tl_memory_arena_create(TLMEBIBYTES(32));
    tl_serializer_read(argv[1]);

    runtime->arenas.frame = tl_memory_arena_create(TLMEBIBYTES(10));
    runtime->engine.ecs.entities = tl_list_create(runtime->arenas.permanent);
    runtime->engine.ecs.components = tl_list_create(runtime->arenas.permanent);

    tl_event_subscribe(TL_EVENT_WINDOW_CREATED, event_echo);

    if (!tl_platform_initialize()) {
        TLERROR("Platform failed to initialize");
        if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
        exit(99);
    }

    if (!tl_application_initialize()) {
        TLERROR("Application failed to intialize");
        if (!tl_application_terminate()) { TLERROR("Application failed to terminate"); }
        if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
        exit(99);
    }

    if (!tl_application_run()) { TLERROR("Application failed to execute"); }

    if (!tl_application_terminate()) { TLERROR("Application failed to terminate"); }

    tl_memory_arena_destroy(runtime->arenas.frame);
    tl_memory_arena_destroy(runtime->arenas.permanent);

    if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }

    TLSTACKPOPV(0)
}