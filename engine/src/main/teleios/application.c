#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/application.h"

b8 tl_application_initialize(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_application_initialize");
    TLDEBUG("Application initialized in %llu micros", tl_profiler_time("tl_application_initialize"));
    tl_profiler_end("tl_application_initialize");
    TLSTACKPOPV(TRUE)
}

b8 tl_application_run(void) {
    TLSTACKPUSH
    TLClock t1, t2; 
    tl_time_clock(&t1);
        
    u64 FPS = 0;
    u64 UPS = 0;

    f64 accumulator = 0.0f;
    f64 lastTime = glfwGetTime();

    //TODO move glClearColor to scene initialization
    glClearColor(0.75f, 0.75f, 0.1f, 1.0f);

    char title[60] = { 0 };
    glfwShowWindow(runtime->platform.window.handle);
    while (!glfwWindowShouldClose(runtime->platform.window.handle)) {
        f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;
        // =========================================================
        // Simulation Pass
        // =========================================================
        accumulator += deltaTime;
        while (accumulator >= runtime->engine.simulation.step) {
            accumulator -= runtime->engine.simulation.step;
            ++UPS;
        }
        // =========================================================
        // Graphics Pass
        // =========================================================
        ++FPS;
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // =========================================================
        // Presenting Pass
        // =========================================================
        glfwSwapBuffers(runtime->platform.window.handle);
        
        tl_time_clock(&t2);
        if (t1.second != t2.second) {
            TLMEMCPY(&t1, &t2, sizeof(TLClock));
            TLDEBUG("FPS %llu, UPS %llu", FPS, UPS);
            FPS = UPS = 0;
        }
        // =========================================================
        // Cleanup Pass
        // =========================================================
        glfwPollEvents();
        tl_memory_arena_reset(runtime->arenas.frame);
    }

    glfwHideWindow(runtime->platform.window.handle);
    TLSTACKPOPV(TRUE)
}

b8 tl_application_terminate(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_application_terminate");
    //-----
    TLDEBUG("Application terminated in %llu micros", tl_profiler_time("tl_application_terminate"));
    tl_profiler_end("tl_application_terminate");
    TLSTACKPOPV(TRUE)
}
