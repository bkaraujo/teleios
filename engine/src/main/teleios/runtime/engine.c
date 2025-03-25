#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

b8 tl_engine_initialize(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_engine_initialize");

    global->application.frame.current = 0;
    global->application.frame.overflow = 0;
    global->application.frame.per_second = 0;

    global->application.simulation.current = 0;
    global->application.simulation.overflow = 0;
    global->application.simulation.per_second = 0;

    tl_memory_arena_reset(global->application.frame.arena);

    TLDEBUG("Engine initialized in %llu micros", tl_profiler_time("tl_engine_initialize"));
    tl_profiler_end("tl_engine_initialize");
    TLSTACKPOPV(TRUE)
}

b8 tl_engine_run(void) {
    TLSTACKPUSH
    TLClock t1, t2;
    tl_time_clock(&t1);

    f64 accumulator = 0.0f;
    f64 lastTime = glfwGetTime();

    glfwShowWindow(global->platform.window.handle);
    while (!glfwWindowShouldClose(global->platform.window.handle)) {
        const f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;

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

    glfwHideWindow(global->platform.window.handle);
    TLSTACKPOPV(TRUE)
}

b8 tl_engine_terminate(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_engine_terminate");

    TLDEBUG("Engine terminated in %llu micros", tl_profiler_time("tl_engine_terminate"));
    tl_profiler_end("tl_engine_terminate");
    TLSTACKPOPV(TRUE)
}
