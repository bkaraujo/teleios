#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static TLGeometry* m_geometry;
static u32 m_shader;

static void tl_renderer_prepare(void) {

    m_geometry = tl_graphics_geometry_create(
        global->allocator,
        1,
        &(TLGeometryAttribute){.name = tl_string_create(global->allocator, "position"), .type = TL_FLOAT3 }
    );

    const u32 indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    tl_graphics_geometry_upload_indices(m_geometry, TL_ARR_LENGTH(indices), indices);

    const f32 vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
       -0.5f, -0.5f, 0.0f,  // bottom left
       -0.5f,  0.5f, 0.0f   // top left
    };
    tl_graphics_geometry_upload_vertices(m_geometry, TL_ARR_LENGTH(vertices), vertices);

    TLString* source = tl_string_create(global->allocator, "assets/shader/basic.glsl");
    m_shader = tl_graphics_shader_load(source);
    tl_string_destroy(source);
}

void tl_scene_load(void) {
    tl_graphics_submit_vna(true, tl_renderer_prepare);
}

static void tl_renderer_cleanup(void) {
    tl_graphics_geometry_destroy(m_geometry);
}

void tl_scene_unload(void) {
    tl_graphics_shader_unload(m_shader);
    tl_graphics_submit_vna(true, tl_renderer_cleanup);
}

static void tl_renderer_clear(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void tl_scene_frame_begin(void) {
    tl_graphics_submit_vna(false, tl_renderer_clear);
}

void tl_scene_step(const f64 step) {
    (void) step;
}

static void tl_renderer_draw(void) {
    glUseProgram(m_shader);
    tl_graphics_geometry_bind(m_geometry);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void tl_scene_update(const f64 delta) {
    (void) delta;
    tl_graphics_submit_vna(false, tl_renderer_draw);
}

static void tl_renderer_swap(void) {
    glfwSwapBuffers(tl_window_handler());
    global->frame_count++;
}

void tl_scene_frame_end(void) {
    tl_graphics_submit_vna(false, tl_renderer_swap);
}