#include "teleios/core.h"
#include "teleios/application.h"

TLRuntime *runtime;

int main (int argc, char *argv[]) {
    TLTRACE(">> main(...)")
    TLINFO("Initializing")
    
    if (argc != 2) { 
        TLTRACE("<< main(...)")
        TLFATAL("argc != 2")
    }

    TLApplication app = {
        .window.resolution = TL_VIDEO_RESOLUTION_HD,
        .window.title = "Teleios APP",
        .simulation.step = 30
    };

    TLRuntime rt = {0};
    runtime = &rt;

    runtime->arena_persistent = tl_memory_arena_create(TLMEBIBYTES(32));

    runtime->platform.window.hovered = FALSE;
    runtime->platform.window.maximized = FALSE;
    runtime->platform.window.minimized = FALSE;
    runtime->platform.window.width = app.window.resolution;
    runtime->platform.window.height = (app.window.resolution * 9) / 16;
    runtime->platform.window.title = tl_string_wrap(runtime->arena_persistent, app.window.title);
    
    runtime->graphics.vsync = FALSE;
    runtime->graphics.wireframe = FALSE;

    runtime->simulation.step = 1.0f / app.simulation.step;

    if (!tl_platform_initialize()) {
        TLERROR("Platform failed to intialize");
        if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
        exit(99);
    }

    runtime->ecs.entities = tl_list_create(runtime->arena_persistent);
    runtime->ecs.components = tl_list_create(runtime->arena_persistent);

    if (!tl_application_initialize()) {
        TLERROR("Application failed to intialize");
        if (!tl_application_terminate()) { TLERROR("Application failed to terminate"); }
        if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
        exit(99);
    }
    
    if (!tl_application_run()) { TLERROR("Application failed to execute"); }
    if (!tl_application_terminate()) { TLERROR("Application failed to terminate"); }
    if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }

    TLINFO("Exiting")
    TLTRACE("<< main(...)")
    return 0;
}