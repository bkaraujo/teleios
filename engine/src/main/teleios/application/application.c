#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/application/loader.h"

b8 tl_application_load(void) {
    TLSTACKPUSH

    if (!tl_scene_load("main")) {
        TLERROR("Failed to load scene [main]");
        TLSTACKPOPV(FALSE)
    }

    // u8 world = tl_layer_create("world");
    // u8 gui = tl_layer_create("gui");

    TLDEBUG("Application initialized in %llu micros", TLPROFILER_MICROS);
    TLSTACKPOPV(TRUE)
}
