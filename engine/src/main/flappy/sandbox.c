#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "teleios/graphics.h"

static u32 m_vao;
static u32 m_shader;

static void tl_renderer_prepare(void) {
    glGenVertexArrays(1, &m_vao); {
        glBindVertexArray(m_vao);

        const f32 vertices[] = {
            0.5f,  0.5f, 0.0f,  // top right
            0.5f, -0.5f, 0.0f,  // bottom right
           -0.5f, -0.5f, 0.0f,  // bottom left
           -0.5f,  0.5f, 0.0f   // top left
        };

        u32 vbo; {
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }

        u32 ebo; {
            glGenBuffers(1, &ebo);

            const u32 indices[] = {  // note that we start from 0!
                0, 1, 3,  // first Triangle
                1, 2, 3   // second Triangle
            };

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        }

        glBindVertexArray(GL_NONE);
    }

    m_shader = glCreateProgram(); {

        const u32 vertexShader = glCreateShader(GL_VERTEX_SHADER); {
            const char *vertexShaderSource = "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;\n"
                "void main()\n"
                "{\n"
                "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                "}\0";
            glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

            i32  success; i8 infoLog[512];
            glCompileShader(vertexShader);
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
            if(!success) {
                glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                TLFATAL("Failed to compile VertexShader\n\n%s", infoLog);
            }

            glAttachShader(m_shader, vertexShader);
        }

        const u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); {
            const char *fragmentShaderSource = "#version 330 core\n"
                "out vec4 FragColor;\n"
                "void main()\n"
                "{\n"
                "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                "}\n\0";
            glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

            i32 success; i8 infoLog[512];
            glCompileShader(fragmentShader);
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
            if(!success) {
                glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                TLFATAL("Failed to compile FragmentShader\n\n%s", infoLog);
            }

            glAttachShader(m_shader, fragmentShader);
        }

        i32 success; i8 infoLog[512];
        glLinkProgram(m_shader);
        glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(m_shader, 512, NULL, infoLog);
            TLFATAL("Failed to link Shader PRogram\n\n%s", infoLog);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
}

void tl_scene_load(void) {
    tl_graphics_submit_vna(false, tl_renderer_prepare);
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