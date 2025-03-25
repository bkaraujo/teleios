#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

b8 tl_scene_load(const char* name) {
    TLSTACKPUSHA("%s", name)
    TLDEBUG("Loading scene [%s]", name);

    if (!tl_serializer_read_scene(name)) {
        TLERROR("Scene [%s] not found", name);
        return FALSE;
    }

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

    TLSTACKPOPV(TRUE)
}
