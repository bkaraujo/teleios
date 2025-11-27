#include "teleios/teleios.h"
#include "teleios/graphics.h"
#include "teleios/graphics/thread.inl"

static TLThread* m_thread;

b8 tl_graphics_initialize(void) {
    TL_PROFILER_PUSH

    m_thread = tl_thread_create(global->allocator, tl_graphics_thread, NULL);
    if (m_thread == NULL) TLFATAL("Failed to create Graphics Thread")

    TL_PROFILER_POP_WITH(true)
}

b8 tl_graphics_terminate(void) {
    TL_PROFILER_PUSH

    if (m_thread) {
        tl_thread_join(m_thread, NULL);
        m_thread = NULL;
    }

    TL_PROFILER_POP_WITH(true)
}