#ifndef __TELEIOS_GRAPHICS_SHADER__
#define __TELEIOS_GRAPHICS_SHADER__

#include "teleios/teleios.h"
#include "glad/glad.h"

#define TL_SHADER_PROGRAM_TYPES 6

typedef struct {
    u32 handle;
    u32 type;
    TLString* source;
} TLShaderSource;

static void tl_graphics_shader_program(TLShaderSource* source) {
    source->handle = glCreateShader(source->type);

    const char* sourcePtr = tl_string_cstr(source->source);
    glShaderSource(source->handle, 1, &sourcePtr, NULL);

    static i32 success;
    static char infoLog[512];

    glCompileShader(source->handle);
    glGetShaderiv(source->handle, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(source->handle, 512, NULL, infoLog);
        TLFATAL("Failed to compile Shader Program: \n\n%s\n\n%s", infoLog, sourcePtr)
    }
}

static u32 tl_graphics_shader_create(TLShaderSource* sources, const u8 count) {
    TLVERBOSE("Creating shader program with %u shaders", count)
    const u32 program = glCreateProgram();

    for (u32 i = 0; i < count; ++i) {
        tl_graphics_shader_program(&sources[i]);
        glAttachShader(program, sources[i].handle);
    }

    glLinkProgram(program);

    static i32 success;
    static char infoLog[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        TLFATAL("Failed to link Shader Program\n\n%s", infoLog)
    }

    for (u32 i = 0; i < count; ++i) {
        glDeleteShader(sources[i].handle);
    }

    return program;
}

static u8 tl_graphics_shader_sources(const TLString* content, TLShaderSource* sources) {
    const struct { const char* name; u32 type; b8 found; } delimiters[] = {
        { "// ::VERTEX",          GL_VERTEX_SHADER,          false },
        { "// ::FRAGMENT",        GL_FRAGMENT_SHADER,        false },
        { "// ::GEOMETRY",        GL_GEOMETRY_SHADER,        false },
        { "// ::TESS_CONTROL",    GL_TESS_CONTROL_SHADER,    false },
        { "// ::TESS_EVALUATION", GL_TESS_EVALUATION_SHADER, false },
        { "// ::COMPUTE",         GL_COMPUTE_SHADER,         false }
    };

    // We scan the content for delimiters.
    typedef struct { i32 file_index; u32 shader_type; } Section;

    u8 section_count = 0;
    Section sections[TL_SHADER_PROGRAM_TYPES] = { 0 };
    for (u8 i = 0; i < TL_SHADER_PROGRAM_TYPES; ++i) {
        if (tl_string_count_of_cstr(content, delimiters[i].name) > 1) {
            TLFATAL("Duplicate shader delimiter found: %s", delimiters[i].name)
        }

        const i32 idx = tl_string_index_of_cstr(content, delimiters[i].name);
        if (idx != -1) {
            // Validate OpenGL version requirements
            if (delimiters[i].type == GL_GEOMETRY_SHADER && GLVersion.major < 3 && GLVersion.minor < 2) {
                 TLFATAL("Geometry shader requires OpenGL 3.2+")
            }
            if ((delimiters[i].type == GL_TESS_CONTROL_SHADER || delimiters[i].type == GL_TESS_EVALUATION_SHADER) && GLVersion.major < 4) {
                 TLFATAL("Tessellation shader requires OpenGL 4.0+")
            }
            if (delimiters[i].type == GL_COMPUTE_SHADER && (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 3))) {
                 TLFATAL("Compute shader requires OpenGL 4.3+")
            }

            sections[section_count].file_index = idx;
            sections[section_count].shader_type = i;
            section_count++;
        }
    }

    if (section_count == 0) {
        return 0;
    }

    // Sort sections by index to extract code between them
    // Bubble sort is fine for 6 elements
    for (u8 i = 0; i < section_count - 1; ++i) {
        for (u8 j = 0; j < section_count - i - 1; ++j) {
            if (sections[j].file_index > sections[j+1].file_index) {
                const Section temp = sections[j];
                sections[j] = sections[j+1];
                sections[j+1] = temp;
            }
        }
    }

    u8 source_count = 0;
    for (u32 i = 0; i < section_count; ++i) {
        i32 start = sections[i].file_index;
        // Skip the delimiter line
        while (start < (i32)tl_string_length(content) && tl_string_char_at(content, start) != '\n') {
            start++;
        }
        start++; // Skip newline

        const i32 end = (i < section_count - 1) ? sections[i+1].file_index : tl_string_length(content);

        if (start >= end) {
             // Empty section or malformed
             continue;
        }

        TLString* source_code = tl_string_substring(content, start, end);
        TLString* trimmed = tl_string_trim(source_code);
        tl_string_destroy(source_code);

        sources[source_count].type = delimiters[sections[i].shader_type].type;
        sources[source_count].source = trimmed;
        source_count++;
    }

    return source_count;
}

u32 tl_graphics_shader_load(const TLString* path) {
    TLVERBOSE("Loading shader from %s", tl_string_cstr(path))
    if (!tl_filesystem_exists(path)) TLFATAL("Shader file not found: %s", tl_string_cstr(path))

    TLString* content = tl_filesystem_read(path);
    if (content == NULL) TLFATAL("Failed to read shader file: %s", tl_string_cstr(path))

    TLShaderSource sources[TL_SHADER_PROGRAM_TYPES] = { 0 };
    const u8 source_count = tl_graphics_shader_sources(content, sources);
    if (source_count == 0) TLFATAL("No shader delimiters found in file: %s", tl_string_cstr(path))

    tl_string_destroy(content);
    
    const u32 program = tl_graphics_shader_create(sources, source_count);
    for (u32 i = 0; i < source_count; ++i) {
        tl_string_destroy(sources[i].source);
    }
    
    return program;
}

void tl_graphics_shader_unload(const u32 shader) {
    if (shader == GL_NONE) return;
    glDeleteProgram(shader);
}

#endif