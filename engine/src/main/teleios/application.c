#include "teleios/core.h"
#include "teleios/application.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

b8 tl_application_initialize(void) {
    TLTRACE(">> tl_application_initialize(void)")

    runtime->simulation.step = 1.0f / 30.0f;

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
    glClearColor(0.75f, 0.75f, 0.1f, 1.0f);

    glfwShowWindow(runtime->platform.window.handle);
    while (!glfwWindowShouldClose(runtime->platform.window.handle)) {
        f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;

        FPS++;
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        accumulator += deltaTime;
        while (accumulator >= runtime->simulation.step) {
            accumulator -= runtime->simulation.step;
            UPS++;
        }

        glfwSwapBuffers(runtime->platform.window.handle);
        
        tl_time_clock(&t2);
        if (t1.second != t2.second) {
            tl_time_clock(&t1);
            TLDEBUG("FPS %llu, UPS %llu", FPS, UPS);
            FPS = UPS = 0;
        }

        glfwPollEvents();
    }

    glfwHideWindow(runtime->platform.window.handle);

    TLTRACE("<< tl_application_run(void)")
    return TRUE;
}

b8 tl_application_terminate(void) {
    TLTRACE(">> tl_application_terminate(void)")

    TLTRACE("<< tl_application_terminate(void)")
    return TRUE;
}