#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static void* tl_renderer_frame_clear(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return NULL;
}

static void* tl_renderer_frame_present(void) {
    glfwSwapBuffers(tl_window_handler());
    return NULL;
}

void tl_renderer_frame_begin(void) {
    TL_PROFILER_PUSH
    tl_graphics_submit_async(tl_renderer_frame_clear);
    TL_PROFILER_POP
}

void tl_renderer_frame_end(void) {
    TL_PROFILER_PUSH
    tl_graphics_submit_sync(tl_renderer_frame_present);
    TL_PROFILER_POP
}