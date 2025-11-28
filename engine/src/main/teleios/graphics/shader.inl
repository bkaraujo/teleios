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

static u32 tl_graphics_shader_load(const TLString* path) {
    TLVERBOSE("Loading shader from %s", tl_string_cstr(path));

    // Open file
    FILE* file = fopen(tl_string_cstr(path), "r");
    if (file == NULL) {
        TLFATAL("Failed to open shader file: %s", tl_string_cstr(path));
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    const u64 fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read entire file
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fclose(file);
        TLFATAL("Failed to allocate memory for shader file");
    }

    const u64 bytesRead = fread(buffer, 1, fileSize, file);
    buffer[bytesRead] = '\0';
    fclose(file);

    // Create allocator for temporary strings
    TLAllocator* allocator = tl_memory_allocator_create(TL_KIBI_BYTES(64), TL_ALLOCATOR_LINEAR);
    TLString* content = tl_string_create(allocator, buffer);
    free(buffer);

    // Find shader delimiters
    const i32 vertexStart = tl_string_index_of_cstr(content, "// ::VERTEX");
    const i32 fragmentStart = tl_string_index_of_cstr(content, "// ::FRAGMENT");

    if (vertexStart == -1) {
        tl_string_destroy(content);
        tl_memory_allocator_destroy(allocator);
        TLFATAL("Shader file missing // ::VERTEX delimiter: %s", tl_string_cstr(path));
    }

    if (fragmentStart == -1) {
        tl_string_destroy(content);
        tl_memory_allocator_destroy(allocator);
        TLFATAL("Shader file missing // ::FRAGMENT delimiter: %s", tl_string_cstr(path));
    }

    // Extract vertex shader (skip the delimiter line)
    i32 vertexCodeStart = vertexStart;
    while (vertexCodeStart < (i32)tl_string_length(content) &&
           tl_string_char_at(content, vertexCodeStart) != '\n') {
        vertexCodeStart++;
    }
    vertexCodeStart++; // Skip newline

    TLString* vertexSource = tl_string_substring(content, vertexCodeStart, fragmentStart);
    TLString* vertexTrimmed = tl_string_trim(vertexSource);
    tl_string_destroy(vertexSource);

    // Extract fragment shader (skip the delimiter line)
    i32 fragmentCodeStart = fragmentStart;
    while (fragmentCodeStart < (i32)tl_string_length(content) &&
           tl_string_char_at(content, fragmentCodeStart) != '\n') {
        fragmentCodeStart++;
    }
    fragmentCodeStart++; // Skip newline

    TLString* fragmentSource = tl_string_substring(content, fragmentCodeStart, tl_string_length(content));
    TLString* fragmentTrimmed = tl_string_trim(fragmentSource);
    tl_string_destroy(fragmentSource);

    // Create shader program
    const u32 shader = tl_graphics_shader_create(vertexTrimmed, fragmentTrimmed);

    // Cleanup
    tl_string_destroy(vertexTrimmed);
    tl_string_destroy(fragmentTrimmed);
    tl_string_destroy(content);
    tl_memory_allocator_destroy(allocator);

    return shader;
}

#endif