#include "teleios/core.h"
#include "teleios/core/platform.h"
#include "teleios/application.h"
#include "GLFW/glfw3.h"

b8 tl_application_initialize(void) {
    TLTRACE(">> tl_application_initialize(void)")
    tl_profiler_begin("tl_application_initialize");
    // ------------
    TLDEBUG("Application initialized in %llu micros", tl_profiler_time("tl_application_initialize"));
    tl_profiler_end("tl_application_initialize");
    TLTRACE("<< tl_application_initialize(void)")
    return TRUE;
}

b8 tl_application_run(void) {
    TLTRACE(">> tl_application_run(void)")
    TLClock t1, t2; 
    tl_time_clock(&t1);
        
    u64 FPS = 0;
    u64 UPS = 0;

    f64 accumulator = 0.0f;
    f64 lastTime = glfwGetTime();

    //TODO move glClearColor to scene initialization
    glClearColor(0.75f, 0.75f, 0.1f, 1.0f);

    glfwShowWindow(runtime->platform.window.handle);
    while (!glfwWindowShouldClose(runtime->platform.window.handle)) {
        f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;
        // =========================================================
        // Simulation Pass
        // =========================================================
        accumulator += deltaTime;
        while (accumulator >= runtime->simulation.step) {
            accumulator -= runtime->simulation.step;
            UPS++;
        }
        // =========================================================
        // Graphics Pass
        // =========================================================
        FPS++;
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // =========================================================
        // Presenting Pass
        // =========================================================
        glfwSwapBuffers(runtime->platform.window.handle);
        
        tl_time_clock(&t2);
        if (t1.second != t2.second) {
            tl_time_clock(&t1);
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

    TLTRACE("<< tl_application_run(void)")
    return TRUE;
}

b8 tl_application_terminate(void) {
    TLTRACE(">> tl_application_terminate(void)")
    tl_profiler_begin("tl_application_terminate");
    //-----
    TLDEBUG("Application terminated in %llu micros", tl_profiler_time("tl_application_terminate"));
    tl_profiler_end("tl_application_terminate");
    TLTRACE("<< tl_application_terminate(void)")
    return TRUE;
}
