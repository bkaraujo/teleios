#include "teleios/teleios.h"

static TLArray* m_scenes;
static TLIterator* m_iterator;

b8 tl_scene_initialize(void) {
    TL_PROFILER_PUSH
    m_scenes = tl_array_create(global->allocator, 4, true);
    m_iterator = tl_array_iterator(m_scenes);
    TL_PROFILER_POP_WITH(true)
}

TLScene * tl_scene_create(void) {
    TL_PROFILER_PUSH
    TLScene* scene = tl_memory_alloc(global->allocator, TL_MEMORY_SCENE, sizeof(TLScene));
    tl_array_push(m_scenes, scene);
    TL_PROFILER_POP_WITH(scene)
}

b8 tl_scene_activate(const TLString* name) {
    TL_PROFILER_PUSH_WITH("name=0x%p", name);
    tl_iterator_resync(m_iterator);
    tl_iterator_rewind(m_iterator);

    while (tl_iterator_has_next(m_iterator)) {
        TLScene* scene = tl_iterator_next(m_iterator);
        if (tl_string_equals_ignore_case(name, scene->name)) {
            global->scene = scene;
            TL_PROFILER_POP_WITH(true)
        }
    }

    TLScene* scene = tl_config_get_scene(name);
    if (scene == NULL) TL_PROFILER_POP_WITH(false)

    tl_array_push(m_scenes, scene);
    global->scene = scene;
    TL_PROFILER_POP_WITH(true)
}

void tl_scene_destroy(const TLScene * scene) {
    TL_PROFILER_PUSH_WITH("scene=0x%p", scene);
    if (!tl_array_remove(m_scenes, (TLScene*) scene)) {
        TLFATAL("Unknown TLScene 0x%p", scene)
    }

    tl_memory_free(global->allocator, (TLScene*) scene);
    TL_PROFILER_POP
}


b8 tl_scene_terminate(void) {
    TL_PROFILER_PUSH
    if (tl_array_size(m_scenes) > 0) {
        TLWARN("Ignoring %d dangling TLScene's", tl_array_size(m_scenes))
    }

    tl_iterator_destroy(m_iterator);
    tl_array_destroy(m_scenes);
    TL_PROFILER_POP_WITH(true)
}