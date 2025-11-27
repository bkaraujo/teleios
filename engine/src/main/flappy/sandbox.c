#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "teleios/graphics.h"

void tl_scene_load(void) {

}

static void tl_renderer_clear(void) {
    global->frame_count++;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void tl_scene_frame_begin(void) {
    tl_graphics_submit_vna(false, tl_renderer_clear);
}

void tl_scene_step(const f64 step) {
    (void) step;
}

void tl_scene_update(const f64 delta) {
    (void) delta;
}

static void tl_renderer_swap(void) {
    glfwSwapBuffers(tl_window_handler());
}

void tl_scene_frame_end(void) {
    tl_graphics_submit_vna(false, tl_renderer_swap);
}