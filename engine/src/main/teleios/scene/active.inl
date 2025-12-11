#ifndef __TELEIOS_SCENE_ACTIVE__
#define __TELEIOS_SCENE_ACTIVE__

#include "teleios/teleios.h"

void tl_scene_load() {
    TL_PROFILER_PUSH
    const TLScene* scene = global->scene;
    tl_scene_execute_script(scene, scene->script_load);

    TL_PROFILER_POP
}

void tl_scene_step(f64 step) {
    // TODO: Implementação futura - lógica de física com timestep fixo
    // Esta função NÃO executa scripts Lua
    // Será uma implementação fixa do engine

    // Por enquanto: no-op
    (void)step;
}

void tl_scene_update(f64 delta) {
    // TODO: Implementação futura - lógica de atualização por frame
    // Esta função NÃO executa scripts Lua
    // Será uma implementação fixa do engine

    // Por enquanto: no-op
    (void)delta;
}

void tl_scene_frame_begin() {
    tl_graphics_clear();
    const TLScene* scene = global->scene;
    tl_scene_execute_script(scene, scene->script_frame_begin);
}

void tl_scene_frame_end() {
    const TLScene* scene = global->scene;
    tl_scene_execute_script(scene, scene->script_frame_end);
    tl_graphics_update();
}

void tl_scene_unload() {
    TL_PROFILER_PUSH

    TLScene* scene = global->scene;
    tl_scene_execute_script(scene, scene->script_unload);

    // Fechar estado Lua após unload
    if (scene->lua_state != NULL) {
        lua_close((lua_State*)scene->lua_state);
        scene->lua_state = NULL;
        TLDEBUG("Closed Lua state for scene '%s'", tl_string_cstr(scene->name));
    }

    TL_PROFILER_POP
}

#endif
