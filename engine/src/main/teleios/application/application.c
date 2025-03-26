#include "teleios/globals.h"
#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/application/loader.h"

b8 tl_application_load(void) {
    TLSTACKPUSH
    // --------------------------------------------------------
    // Load the desired scene
    // --------------------------------------------------------
    if (!tl_scene_load("main")) {
        TLERROR("Failed to load scene [main]");
        TLSTACKPOPV(FALSE)
    }
    // --------------------------------------------------------
    // Apply the scene's settings
    // --------------------------------------------------------
    glClearColor(
        global->application.scene.graphics.clear_color.x,
        global->application.scene.graphics.clear_color.y,
        global->application.scene.graphics.clear_color.z,
        global->application.scene.graphics.clear_color.w
    );

    if (global->application.scene.graphics.depth_enabled) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(global->application.scene.graphics.depth_function);
    }

    if (global->application.scene.graphics.blend_enabled) {
        glEnable(GL_BLEND);
        glBlendEquation(global->application.scene.graphics.blend_equation);
        glBlendFunc(
            global->application.scene.graphics.blend_function_src,
            global->application.scene.graphics.blend_function_tgt
        );
    }
    // u8 world = tl_layer_create("world");
    // u8 gui = tl_layer_create("gui");

    TLDEBUG("Application initialized in %llu micros", TLPROFILER_MICROS);
    TLSTACKPOPV(TRUE)
}
