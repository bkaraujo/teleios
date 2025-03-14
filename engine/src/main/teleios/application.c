#include "teleios/core.h"
#include "teleios/application.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef TLPLATFORM_APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(640, 480, "Teleios APP", NULL, NULL);
    if (window == NULL) {
        TLERROR("Failed to create GLFW window");
        return FALSE;       
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("Failed to initialize GLAD")
        return FALSE;
    }

    glfwSwapInterval(0);

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

    glfwShowWindow(window);
    while (!glfwWindowShouldClose(window)) {
        f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;

        FPS++;
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        accumulator += deltaTime;
        while (accumulator >= STEP) {
            accumulator -= STEP;
            UPS++;
        }

        glfwSwapBuffers(window);
        
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