#ifndef __TELEIOS_GRAPHICS_UTILS__
#define __TELEIOS_GRAPHICS_UTILS__

#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// #####################################################################################################################
// tl_graphics_clear
// #####################################################################################################################
static void tl_renderer_clear(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void tl_graphics_clear(void) {
    tl_graphics_submit_vna(false, tl_renderer_clear);
}

// #####################################################################################################################
// tl_graphics_update
// #####################################################################################################################
static void tl_renderer_swap(void) {
    glfwSwapBuffers(tl_window_handler());
    global->frame_count++;
}

void tl_graphics_update(void) {
    tl_graphics_submit_vna(false, tl_renderer_swap);
}

#endif