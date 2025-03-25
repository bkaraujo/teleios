#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/application.h"
#include "teleios/globals.h"

b8 tl_application_initialize(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_application_initialize");

    if (!tl_scene_load("main")) {
        TLERROR("Failed to load scene [main]");
        TLSTACKPOPV(FALSE)
    }

    TLDEBUG("Application initialized in %llu micros", tl_profiler_time("tl_application_initialize"));
    tl_profiler_end("tl_application_initialize");
    TLSTACKPOPV(TRUE)
}

b8 tl_application_run(void) {
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

        // =========================================================
        // Simulation Pass
        // =========================================================
        accumulator += deltaTime;
        while (accumulator >= global->application.simulation.step) {
            global->application.simulation.current++;
            global->application.simulation.per_second++;

            accumulator -= global->application.simulation.step;
        }
        // =========================================================
        // Graphics Pass
        // =========================================================
        global->application.frame.per_second++;
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // =========================================================
        // Presenting Pass
        // =========================================================
        glfwSwapBuffers(global->platform.window.handle);
        
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

b8 tl_application_terminate(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_application_terminate");

    TLDEBUG("Application terminated in %llu micros", tl_profiler_time("tl_application_terminate"));
    tl_profiler_end("tl_application_terminate");
    TLSTACKPOPV(TRUE)
}
