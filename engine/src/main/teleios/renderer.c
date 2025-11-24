#include "teleios/teleios.h"
#include "teleios/graphics/types.inl"
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

// External function from cmdbuffer.inl (defined in graphics.c)
extern void tl_cmdbuffer_record(void* (*func_no_args)(void), void* (*func_with_args)(void**), void** args, u32 args_count);

void tl_renderer_frame_begin(void) {
    TL_PROFILER_PUSH

    // Begin recording to a new command buffer
    tl_cmdbuffer_begin();

    // Record the clear command
    tl_cmdbuffer_record(tl_renderer_frame_clear, NULL, NULL, 0);

    TL_PROFILER_POP
}

void tl_renderer_frame_end(void) {
    TL_PROFILER_PUSH

    // Record the present command
    tl_cmdbuffer_record(tl_renderer_frame_present, NULL, NULL, 0);

    // End recording and submit buffer for async execution
    tl_cmdbuffer_end();

    TL_PROFILER_POP
}