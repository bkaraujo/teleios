#include "teleios/core.h"
#include "teleios/core/platform.h"
#include "teleios/application.h"
#include "teleios/core/serializer.h"

TLRuntime *runtime;

int main (const int argc, const char *argv[]) {
    TLTRACE(">> main(...)")
    TLINFO("Initializing")

    if (argc != 2) {
        TLTRACE("<< main(...)")
        TLFATAL("argc != 2")
    }

    TLRuntime rt = {0};

    runtime = &rt;
    runtime->arenas.permanent = tl_memory_arena_create(TLMEBIBYTES(32));
    runtime->arenas.frame = tl_memory_arena_create(TLMEBIBYTES(10));

    tl_serializer_read(argv[1]);

    runtime->ecs.entities = tl_list_create(runtime->arenas.permanent);
    runtime->ecs.components = tl_list_create(runtime->arenas.permanent);

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

    TLINFO("Exiting")
    TLTRACE("<< main(...)")
    return 0;
}