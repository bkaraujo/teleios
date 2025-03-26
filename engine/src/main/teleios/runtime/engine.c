#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

static b8 running = TRUE;
static b8 paused = FALSE;

static TLEventStatus tl_process_window_minimized(const TLEvent *event) {
    paused = TRUE;

    global->application.frame.per_second = 0;
    global->application.simulation.per_second = 0;
    TLINFO("Simulation paused")
    return TL_EVENT_NOT_CONSUMED;
}

static TLEventStatus tl_process_window_closed(const TLEvent *event) {
    running = FALSE;
    return TL_EVENT_NOT_CONSUMED;
}

static TLEventStatus tl_process_window_restored(const TLEvent *event) {
    paused = FALSE;
    TLINFO("Simulation resumed")
    return TL_EVENT_NOT_CONSUMED;
}

b8 tl_engine_initialize(void) {
    TLSTACKPUSH

    global->application.frame.current = 0;
    global->application.frame.overflow = 0;
    global->application.frame.per_second = 0;

    global->application.simulation.current = 0;
    global->application.simulation.overflow = 0;
    global->application.simulation.per_second = 0;

    tl_memory_arena_reset(global->application.frame.arena);

    tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_process_window_closed);
    tl_event_subscribe(TL_EVENT_WINDOW_RESTORED, tl_process_window_restored);
    tl_event_subscribe(TL_EVENT_WINDOW_MINIMIZED, tl_process_window_minimized);


    TLDEBUG("Engine initialized in %llu micros", TLPROFILER_MICROS);
    TLSTACKPOPV(TRUE)
}

b8 tl_engine_run(void) {
    TLSTACKPUSH
    TLClock t1, t2;
    tl_time_clock(&t1);

    f64 accumulator = 0.0f;
    f64 lastTime = glfwGetTime();

    glfwShowWindow(global->platform.window.handle);
    while (running) {
        const f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;

        if (!paused) {
            global->application.frame.current++;
            if (global->application.frame.current == 0) {
                global->application.frame.overflow++;
                TLWARN("global->application.frame.overflow = %u", global->application.frame.overflow)
            }

            // =========================================================
            // Simulation Pass
            // =========================================================
            accumulator += deltaTime;
            while (accumulator >= global->application.simulation.step) {
                global->application.simulation.current++;
                if (global->application.simulation.current == 0) {
                    global->application.simulation.overflow++;
                    TLWARN("global->application.simulation.overflow = %u", global->application.simulation.overflow)
                }

                global->application.simulation.per_second++;

                accumulator -= global->application.simulation.step;
            }
            // =========================================================
            // Update Pass
            // =========================================================
            global->application.frame.per_second++;
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // =========================================================
            // Rendering Pass
            // =========================================================
        }

        // =========================================================
        // Presentation Pass
        // =========================================================
        glfwSwapBuffers(global->platform.window.handle);

        // =========================================================
        // FPS calculation
        // =========================================================
        tl_time_clock(&t2);
        if (t1.second != t2.second) {
            TLMEMCPY(&t1, &t2, sizeof(TLClock));
            TLDEBUG("FPS %llu, UPS %llu", global->application.frame.per_second, global->application.simulation.per_second);
            global->application.frame.per_second = global->application.simulation.per_second = 0;
        }
        // =========================================================
        // Cleanup Pass
        // =========================================================
        glfwPollEvents();
        tl_memory_arena_reset(global->application.frame.arena);
    }

    global->application.frame.per_second = 0;
    global->application.simulation.per_second = 0;
    glfwHideWindow(global->platform.window.handle);

    TLSTACKPOPV(TRUE)
}

b8 tl_engine_terminate(void) {
    TLSTACKPUSH

    TLDEBUG("Engine terminated in %llu micros", TLPROFILER_MICROS);
    TLSTACKPOPV(TRUE)
}
