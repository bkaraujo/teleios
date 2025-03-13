#include "teleios/application.h"
#include "teleios/logger.h"
#include "teleios/container.h"
#include "teleios/platform.h"
#include "teleios/memory.h"
#include "GLFW/glfw3.h"
#include "teleios/time.h"

static GLFWwindow* window;
static f64 STEP = 1.0f / 30.0f;

b8 tl_application_initialize(void) {
    TLTRACE(">> tl_application_initialize(void)")

    TLVERBOSE("Initializing GLFW");
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        return FALSE;
    }
    
    TLVERBOSE("Creating window");
    window = glfwCreateWindow(640, 480, "Teleios APP", NULL, NULL);
    if (window == NULL) {
        TLERROR("Failed to create GLFW window");
        return FALSE;       
    }

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
    
    glfwShowWindow(window);
    while (!glfwWindowShouldClose(window)) {
        f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;

        FPS++;

        accumulator += deltaTime;
        while (accumulator >= STEP) {
            accumulator -= STEP;
            UPS++;
        }

        tl_time_clock(&t2);
        if (t1.second != t2.second) {
            tl_time_clock(&t1);
            TLDEBUG("FPS %llu, UPS %llu", FPS, UPS);
            FPS = UPS = 0;
        }

        glfwPollEvents();
    }

    glfwHideWindow(window);

    TLTRACE("<< tl_application_run(void)")
    return TRUE;
}

b8 tl_application_terminate(void) {
    TLTRACE(">> tl_application_terminate(void)")

    TLVERBOSE("Terminating GLFW");
    glfwTerminate();

    TLTRACE("<< tl_application_terminate(void)")
    return TRUE;
}