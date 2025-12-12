#ifndef __TELEIOS_SCENE_LIFECYCLE__
#define __TELEIOS_SCENE_LIFECYCLE__

#include "teleios/teleios.h"

TLScene* tl_scene_create(void) {
    TL_PROFILER_PUSH
    TLScene* scene = tl_memory_alloc(global->allocator, TL_MEMORY_SCENE, sizeof(TLScene));
    tl_array_push(m_scenes, scene);
    TL_PROFILER_POP_WITH(scene)
}

b8 tl_scene_activate(const TLString* name) {
    TL_PROFILER_PUSH_WITH("name=0x%p", name);
    TLINFO("Activating scene '%s'", tl_string_cstr(name))

    if (global->scene != NULL) {
        tl_scene_unload(global->scene);
        global->scene = NULL;
    }

    // 1. Verificar se cena já está carregada
    tl_iterator_resync(m_iterator);
    tl_iterator_rewind(m_iterator);
    while (tl_iterator_has_next(m_iterator)) {
        TLScene* scene = tl_iterator_next(m_iterator);
        if (tl_string_equals_ignore_case(name, scene->name)) {
            tl_scene_apply_graphics_config(scene);
            tl_scene_load(scene);
            TL_PROFILER_POP_WITH(true)
        }
    }

    TLScene* scene = tl_config_get_scene(name);
    if (scene == NULL) TL_PROFILER_POP_WITH(false)

    tl_scene_init_lua(scene);
    tl_scene_apply_graphics_config(scene);
    tl_scene_apply_camera_config(scene);
    tl_array_push(m_scenes, scene);
    tl_scene_load(scene);
    global->scene = scene;

    TLDEBUG("Scene '%s' activated", tl_string_cstr(name));
    TL_PROFILER_POP_WITH(true)
}

void tl_scene_destroy(const TLScene* scene) {
    TL_PROFILER_PUSH_WITH("scene=0x%p", scene);

    // Executar script de unload antes de destruir
    if (scene->lua_state != NULL) {
        tl_scene_unload((TLScene*)scene);
    }

    // Remove do array caso esteja lá
    tl_array_remove(m_scenes, (TLScene*)scene);

    // Liberar strings
    if (scene->name) tl_string_destroy(scene->name);
    if (scene->script_load) tl_string_destroy(scene->script_load);
    if (scene->script_unload) tl_string_destroy(scene->script_unload);
    if (scene->script_frame_begin) tl_string_destroy(scene->script_frame_begin);
    if (scene->script_frame_end) tl_string_destroy(scene->script_frame_end);
    if (scene->allocator) tl_memory_allocator_destroy(scene->allocator);

    tl_memory_free(global->allocator, (TLScene*)scene);
    TL_PROFILER_POP
}

#endif