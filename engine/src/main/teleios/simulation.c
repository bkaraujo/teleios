#include "teleios/teleios.h"
#include "teleios/simulation.h"

static void* tl_simulation_thread(void*);
static TLThread* m_thread;

b8 tl_simulation_initialize(void) {
    TL_PROFILER_PUSH

    m_thread = tl_thread_create(global->allocator, tl_simulation_thread, NULL);
    if (m_thread == NULL) TLFATAL("Failed to create Simulation Thread")

    TL_PROFILER_POP_WITH(true)
}

static void* tl_simulation_thread(void* _) {
    (void) _;
    TLDEBUG("Entering Main Loop")
    for ( ; global->running ; ) {

    }
    TLDEBUG("Finalizando")
    return NULL;
}

b8 tl_simulation_terminate(void) {
    TL_PROFILER_PUSH

    if (m_thread) {
        tl_thread_join(m_thread, NULL);
        m_thread = NULL;
    }

    TL_PROFILER_POP_WITH(true)
}