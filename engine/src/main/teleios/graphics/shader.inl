#ifndef __TELEIOS_GRAPHICS_SHADER__
#define __TELEIOS_GRAPHICS_SHADER__

#include "teleios/teleios.h"
#include "glad/glad.h"

static u32 tl_graphics_shader_program(const u32 type, const TLString* source) {
    const u32 program = glCreateShader(type);

    const char* sourcePtr = tl_string_cstr(source);
    glShaderSource(program, 1, &sourcePtr, NULL);

    static i32 success;
    static char infoLog[512];

    glCompileShader(program);
    glGetShaderiv(program, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        TLFATAL("Failed to compile Shader Program: \n\n%s\n\n%s", infoLog, sourcePtr);
    }

    return program;
}

static u32 tl_graphics_shader_create(const TLString* vertex, const TLString* fragment) {
    TLVERBOSE("Creating shader")
    const u32 shader = glCreateProgram();

    const u32 vertexShader = tl_graphics_shader_program(GL_VERTEX_SHADER, vertex);
    const u32 fragmentShader = tl_graphics_shader_program(GL_FRAGMENT_SHADER, fragment);

    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    static i32 success;
    static char infoLog[512];

    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shader, 512, NULL, infoLog);
        TLFATAL("Failed to link Shader Program\n\n%s", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shader;
}

#endif