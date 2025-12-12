#ifndef __TELEIOS_SCENE_ACTIVE__
#define __TELEIOS_SCENE_ACTIVE__

#include "teleios/teleios.h"

void tl_scene_frame_begin() {
    tl_graphics_clear();
    const TLScene* scene = global->scene;
    tl_scene_execute_script(scene, scene->script_frame_begin);
}

void tl_scene_step(const f64 step) {
    // TODO: Implementação futura - lógica de física com timestep fixo
    // Esta função NÃO executa scripts Lua
    // Será uma implementação fixa do engine

    // Por enquanto: no-op
    (void)step;
}

void tl_scene_update(const f64 delta) {
    // TODO: Implementação futura - lógica de atualização por frame
    // Esta função NÃO executa scripts Lua
    // Será uma implementação fixa do engine

    // Por enquanto: no-op
    (void)delta;
}

void tl_scene_frame_end() {
    const TLScene* scene = global->scene;
    tl_scene_execute_script(scene, scene->script_frame_end);
    tl_graphics_update();
}

#endif
