#include "teleios/teleios.h"

static TLArray* m_scenes = NULL;
static TLIterator* m_iterator = NULL;

b8 tl_scene_initialize(void) {
    TL_PROFILER_PUSH
    m_scenes = tl_array_create(global->allocator, 3, true);
    m_iterator = tl_array_iterator(m_scenes);
    TL_PROFILER_POP_WITH(true)
}

#include "teleios/scene/private.inl"
#include "teleios/scene/lifecycle.inl"
#include "teleios/scene/active.inl"

b8 tl_scene_terminate(void) {
    TL_PROFILER_PUSH
    
    const unsigned size = tl_array_size(m_scenes);
    for (unsigned i = 0 ; i < size; ++i) {
        tl_scene_destroy(tl_array_get(m_scenes, i));
    }

    tl_iterator_destroy(m_iterator);
    tl_array_destroy(m_scenes);
    TL_PROFILER_POP_WITH(true)
}
