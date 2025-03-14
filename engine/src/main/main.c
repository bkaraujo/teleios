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

    TLRuntime rt = {0};
    runtime = &rt;

    runtime->platform.window.hovered = FALSE;
    runtime->platform.window.maximized = FALSE;
    runtime->platform.window.minimized = FALSE;
    runtime->platform.window.width = 640;
    runtime->platform.window.height = 480;
    runtime->platform.window.title = "Teleios App";
    
    runtime->graphics.vsync = FALSE;
    runtime->graphics.wireframe = FALSE;

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