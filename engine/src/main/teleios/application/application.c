#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/application/application.h"

b8 tl_application_initialize(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_application_initialize");

    if (!tl_scene_load("main")) {
        TLERROR("Failed to load scene [main]");
        TLSTACKPOPV(FALSE)
    }

    // u8 world = tl_layer_create("world");
    // u8 gui = tl_layer_create("gui");

    TLDEBUG("Application initialized in %llu micros", tl_profiler_time("tl_application_initialize"));
    tl_profiler_end("tl_application_initialize");
    TLSTACKPOPV(TRUE)
}


b8 tl_application_terminate(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_application_terminate");

    TLDEBUG("Application terminated in %llu micros", tl_profiler_time("tl_application_terminate"));
    tl_profiler_end("tl_application_terminate");
    TLSTACKPOPV(TRUE)
}
