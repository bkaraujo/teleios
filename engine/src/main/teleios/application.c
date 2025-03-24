#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/application.h"
#include "teleios/globals.h"

static TLScene *scene;

b8 tl_application_initialize(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_application_initialize");
    scene = tl_scene_create("main");

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

    glfwShowWindow(global->window.handle);
    while (!glfwWindowShouldClose(global->window.handle)) {
        f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;
        // =========================================================
        // Simulation Pass
        // =========================================================
        accumulator += deltaTime;
        while (accumulator >= global->simulation.step) {
            accumulator -= global->simulation.step;
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
        glfwSwapBuffers(global->window.handle);
        
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
        tl_memory_arena_reset(global->arenas.frame);
    }

    glfwHideWindow(global->window.handle);
    TLSTACKPOPV(TRUE)
}

b8 tl_application_terminate(void) {
    TLSTACKPUSH
    tl_profiler_begin("tl_application_terminate");
    if (scene != NULL) {
        tl_memory_arena_destroy(scene->arena);
        scene->arena = NULL;
    }

    TLDEBUG("Application terminated in %llu micros", tl_profiler_time("tl_application_terminate"));
    tl_profiler_end("tl_application_terminate");
    TLSTACKPOPV(TRUE)
}
