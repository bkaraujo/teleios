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
    TLDEBUG("Initializing")
    const f64 FRAME_CAP = 250000.0;     // Cap at 250ms to prevent spiral of death
    
    TLString* sStep = tl_config_get("teleios.simulation.step");
    const f32 fStep = tl_number_f32_from_string(sStep);
    const f64 STEP = 1000000.0 / (fStep == 0 ? 10 : fStep);
    
    f64 accumulator = 0.0;
    u64 last_time = tl_time_epoch_micros();
    
    TLDEBUG("Entering Main Loop")
    for ( ; global->running ; ) {
        const u64 new_time = tl_time_epoch_micros();
        f64 delta_time = (f64)(new_time - last_time);
        last_time = new_time;
        
        if (!global->suspended) {

            tl_input_snapshot();
            global->update_count++;

            // Cap frame time to prevent spiral of death
            if (delta_time > FRAME_CAP) {
                TLWARN("Frame time %.2f ms exceeded, capping to %.2f ms",  delta_time / 1000.0, FRAME_CAP / 1000.0);
                delta_time = FRAME_CAP;
            }
            
            accumulator += delta_time;
            while (accumulator >= STEP) {
                // update(STEP);
                accumulator -= STEP;
            }
            
            const f64 alpha = accumulator / STEP;
            // render(alpha);
            (void)alpha;  // Suppress unused variable warning
        }
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