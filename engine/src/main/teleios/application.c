#include "teleios/teleios.h"
#include "teleios/application.h"
#include <GLFW/glfw3.h>

// ---------------------------------
// Module State
// ---------------------------------

static u64 frame_count = 0;
static u64 update_count = 0;
static b8 m_running = false;
static b8 m_paused = false;

// ---------------------------------
// Event Handlers
// ---------------------------------

static TLEventStatus tl_application_handle_window_minimized(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)

    m_paused = true;

    frame_count = 0;
    update_count = 0;
    TLINFO("Simulation paused")

    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_application_handle_window_closed(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)

    m_running = false;

    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_application_handle_window_restored(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)

    m_paused = false;
    TLINFO("Simulation resumed")

    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

b8 tl_application_initialize(void) {
    TL_PROFILER_PUSH

    tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_application_handle_window_closed);
    tl_event_subscribe(TL_EVENT_WINDOW_RESTORED, tl_application_handle_window_restored);
    tl_event_subscribe(TL_EVENT_WINDOW_MINIMIZED, tl_application_handle_window_minimized);
    
    TL_PROFILER_POP_WITH(true)
}

b8 tl_application_run(void) {
    TL_PROFILER_PUSH

    const f64 STEP = 1000000.0 / 60.0;  // ~16666.67 µs
    const f64 FRAME_CAP = 250000.0;  // Cap at 250ms to prevent spiral of death

    f64 accumulator = 0.0;
    u64 last_time = tl_time_epoch_micros();

    // FPS/UPS counters
    f64 fps_timer = 0.0;

    m_running = true;
    glfwShowWindow(tl_window_handler());
    while (m_running) {
        const u64 new_time = tl_time_epoch_micros();
        f64 delta_time = (f64)(new_time - last_time);
        last_time = new_time;

        if (!m_paused) {
            // Cap frame time to prevent spiral of death
            if (delta_time > FRAME_CAP) {
                TLWARN("Frame time %.2f ms exceeded, capping to %.2f ms", delta_time / 1000.0, FRAME_CAP / 1000.0);
                delta_time = FRAME_CAP;
            }

            tl_renderer_frame_begin();

            fps_timer += delta_time;
            accumulator += delta_time;
            while (accumulator >= STEP) {
                // update(STEP);
                update_count++;
                accumulator -= STEP;
            }

            const f64 alpha = accumulator / STEP;
            // render(alpha);
            (void)alpha;  // Suppress unused variable warning until render is implemented
        }

        // Submit rendering work to graphics thread (asynchronous - returns immediately)
        tl_renderer_frame_end();

        // Poll events on main thread (GLFW requirement)
        glfwPollEvents();

        frame_count++;
        if (fps_timer >= ONE_SECOND_MICROS) {
            static char buffer[60];
            snprintf(buffer, sizeof(buffer), "FPS: %llu | UPS: %llu", frame_count, update_count);
            glfwSetWindowTitle(tl_window_handler(), buffer);

            frame_count = update_count = 0;
            fps_timer -= ONE_SECOND_MICROS;
        }
    }

    glfwHideWindow(tl_window_handler());
    TL_PROFILER_POP_WITH(true)
}

b8 tl_application_terminate(void) {
    TL_PROFILER_PUSH
    
    TL_PROFILER_POP_WITH(true)
}