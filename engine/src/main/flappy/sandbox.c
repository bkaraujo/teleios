#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "teleios/graphics/shader.inl"

static u32 m_vao;
static u32 m_shader;

static void tl_renderer_prepare(void) {
    glCreateVertexArrays(1, &m_vao); {

        u32 vbo; {
            glCreateBuffers(1, &vbo);
            glVertexArrayVertexBuffer(m_vao, 0, vbo, 0, 3 * sizeof(float));
            glEnableVertexArrayAttrib(m_vao, 0);
            glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
            glVertexArrayAttribBinding(m_vao, 0, 0);

            const f32 vertices[] = {
                0.5f,  0.5f, 0.0f,  // top right
                0.5f, -0.5f, 0.0f,  // bottom right
               -0.5f, -0.5f, 0.0f,  // bottom left
               -0.5f,  0.5f, 0.0f   // top left
            };

            glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
        }

        u32 ebo; {
            glCreateBuffers(1, &ebo);
            glVertexArrayElementBuffer(m_vao, ebo);

            const u32 indices[] = {  // note that we start from 0!
                0, 1, 3,  // first Triangle
                1, 2, 3   // second Triangle
            };

            glNamedBufferData(ebo, sizeof(indices), indices, GL_STATIC_DRAW);
        }
    }

    TLString* source = tl_string_create(global->allocator, "assets/shader/basic.glsl");
    m_shader = tl_graphics_shader_load(source);
    tl_string_destroy(source);
}

void tl_scene_load(void) {
    tl_graphics_submit_vna(true, tl_renderer_prepare);
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
    glBindVertexArray(m_vao);
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