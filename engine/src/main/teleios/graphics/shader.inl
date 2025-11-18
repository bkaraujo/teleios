#ifndef __TELEIOS_GRAPHICS_SHADER__
#define __TELEIOS_GRAPHICS_SHADER__

#include <stdarg.h>
#include <stdio.h>
#include "glad/glad.h"
#include "teleios/graphics/types.inl"
#include "teleios/teleios.h"

// ---------------------------------
// Helper Functions
// ---------------------------------

/**
 * @brief Read shader source code from file
 * @param path Path to shader file
 * @return Heap-allocated string containing shader source code (must be freed)
 */
static char* tl_shader_read_file(const char* path) {
    TL_PROFILER_PUSH_WITH("path=%s", path)

    FILE* file = fopen(path, "rb");
    if (!file) {
        TLERROR("Failed to open shader file: %s", path)
        TL_PROFILER_POP_WITH(NULL)
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer
    char* buffer = (char*)tl_memory_alloc(m_allocator, TL_MEMORY_GRAPHICS, size + 1);

    // Read file
    size_t read = fread(buffer, 1, size, file);
    buffer[read] = '\0';
    fclose(file);

    TLDEBUG("Loaded shader source from %s (%ld bytes)", path, size)
    TL_PROFILER_POP_WITH(buffer)
}

/**
 * @brief Compile a single shader stage
 * @param type OpenGL shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
 * @param source Shader source code
 * @return OpenGL shader object ID, or 0 on failure
 */
static u32 tl_shader_compile(u32 type, const char* source) {
    TL_PROFILER_PUSH_WITH("type=%u", type)

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        TLERROR("Shader compilation failed:\n%s", info_log)
        glDeleteShader(shader);
        TL_PROFILER_POP_WITH(0)
    }

    TLDEBUG("Shader compiled successfully (ID: %u)", shader)
    TL_PROFILER_POP_WITH(shader)
}

/**
 * @brief Link multiple shader stages into a program
 * @param shaders Array of compiled shader IDs
 * @param count Number of shaders
 * @return OpenGL program ID, or 0 on failure
 */
static u32 tl_shader_link(const u32* shaders, u32 count) {
    TL_PROFILER_PUSH_WITH("count=%u", count)

    GLuint program = glCreateProgram();

    // Attach all shaders
    for (u32 i = 0; i < count; i++) {
        glAttachShader(program, shaders[i]);
    }

    glLinkProgram(program);

    // Check link status
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        TLERROR("Shader program linking failed:\n%s", info_log)
        glDeleteProgram(program);
        TL_PROFILER_POP_WITH(0)
    }

    // Detach and delete shaders (no longer needed after linking)
    for (u32 i = 0; i < count; i++) {
        glDetachShader(program, shaders[i]);
        glDeleteShader(shaders[i]);
    }

    TLDEBUG("Shader program linked successfully (ID: %u)", program)
    TL_PROFILER_POP_WITH(program)
}

/**
 * @brief Upload a single uniform to GPU
 * @param program OpenGL program ID
 * @param name Uniform name
 * @param uniform Uniform data
 * @return true on success, false on failure
 */
static b8 tl_shader_upload_uniform(u32 program, const char* name, const TLShaderUniform* uniform) {
    TL_PROFILER_PUSH_WITH("name=%s, type=%d", name, uniform->type)

    const GLint location = glGetUniformLocation(program, name);
    if (location == -1) {
        TLWARN("Uniform '%s' not found in shader program", name)
        TL_PROFILER_POP_WITH(false)
    }

    switch (uniform->type) {
        // Unsigned byte types
        case TL_BUFFER_UBYTE1: glUniform1ui(location, (GLuint)uniform->value.u64[0]); break;
        case TL_BUFFER_UBYTE2: glUniform2ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1]); break;
        case TL_BUFFER_UBYTE3: glUniform3ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1], (GLuint)uniform->value.u64[2]); break;
        case TL_BUFFER_UBYTE4: glUniform4ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1], (GLuint)uniform->value.u64[2], (GLuint)uniform->value.u64[3]); break;

        // Unsigned short types
        case TL_BUFFER_USHORT1: glUniform1ui(location, (GLuint)uniform->value.u64[0]); break;
        case TL_BUFFER_USHORT2: glUniform2ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1]); break;
        case TL_BUFFER_USHORT3: glUniform3ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1], (GLuint)uniform->value.u64[2]); break;
        case TL_BUFFER_USHORT4: glUniform4ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1], (GLuint)uniform->value.u64[2], (GLuint)uniform->value.u64[3]); break;

        // Unsigned int types
        case TL_BUFFER_UINT1: glUniform1ui(location, (GLuint)uniform->value.u64[0]); break;
        case TL_BUFFER_UINT2: glUniform2ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1]); break;
        case TL_BUFFER_UINT3: glUniform3ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1], (GLuint)uniform->value.u64[2]); break;
        case TL_BUFFER_UINT4: glUniform4ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1], (GLuint)uniform->value.u64[2], (GLuint)uniform->value.u64[3]); break;

        // Unsigned long types
        case TL_BUFFER_ULONG1: glUniform1ui(location, (GLuint)uniform->value.u64[0]); break;
        case TL_BUFFER_ULONG2: glUniform2ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1]); break;
        case TL_BUFFER_ULONG3: glUniform3ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1], (GLuint)uniform->value.u64[2]); break;
        case TL_BUFFER_ULONG4: glUniform4ui(location, (GLuint)uniform->value.u64[0], (GLuint)uniform->value.u64[1], (GLuint)uniform->value.u64[2], (GLuint)uniform->value.u64[3]); break;

        // Signed byte types
        case TL_BUFFER_BYTE1: glUniform1i(location, (GLint)uniform->value.i64[0]); break;
        case TL_BUFFER_BYTE2: glUniform2i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1]); break;
        case TL_BUFFER_BYTE3: glUniform3i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1], (GLint)uniform->value.i64[2]); break;
        case TL_BUFFER_BYTE4: glUniform4i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1], (GLint)uniform->value.i64[2], (GLint)uniform->value.i64[3]); break;

        // Signed short types
        case TL_BUFFER_SHORT1: glUniform1i(location, (GLint)uniform->value.i64[0]); break;
        case TL_BUFFER_SHORT2: glUniform2i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1]); break;
        case TL_BUFFER_SHORT3: glUniform3i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1], (GLint)uniform->value.i64[2]); break;
        case TL_BUFFER_SHORT4: glUniform4i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1], (GLint)uniform->value.i64[2], (GLint)uniform->value.i64[3]); break;

        // Signed int types
        case TL_BUFFER_INT1: glUniform1i(location, (GLint)uniform->value.i64[0]); break;
        case TL_BUFFER_INT2: glUniform2i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1]); break;
        case TL_BUFFER_INT3: glUniform3i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1], (GLint)uniform->value.i64[2]); break;
        case TL_BUFFER_INT4: glUniform4i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1], (GLint)uniform->value.i64[2], (GLint)uniform->value.i64[3]); break;

        // Signed long types
        case TL_BUFFER_LONG1: glUniform1i(location, (GLint)uniform->value.i64[0]); break;
        case TL_BUFFER_LONG2: glUniform2i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1]); break;
        case TL_BUFFER_LONG3: glUniform3i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1], (GLint)uniform->value.i64[2]); break;
        case TL_BUFFER_LONG4: glUniform4i(location, (GLint)uniform->value.i64[0], (GLint)uniform->value.i64[1], (GLint)uniform->value.i64[2], (GLint)uniform->value.i64[3]); break;

        // Float types
        case TL_BUFFER_FLOAT1: glUniform1f(location, (GLfloat)uniform->value.f64[0]); break;
        case TL_BUFFER_FLOAT2: glUniform2f(location, (GLfloat)uniform->value.f64[0], (GLfloat)uniform->value.f64[1]); break;
        case TL_BUFFER_FLOAT3: glUniform3f(location, (GLfloat)uniform->value.f64[0], (GLfloat)uniform->value.f64[1], (GLfloat)uniform->value.f64[2]); break;
        case TL_BUFFER_FLOAT4: glUniform4f(location, (GLfloat)uniform->value.f64[0], (GLfloat)uniform->value.f64[1], (GLfloat)uniform->value.f64[2], (GLfloat)uniform->value.f64[3]); break;

        // Double types (cast to float as OpenGL doesn't have native double uniforms in older versions)
        case TL_BUFFER_DOUBLE1: glUniform1f(location, (GLfloat)uniform->value.f64[0]); break;
        case TL_BUFFER_DOUBLE2: glUniform2f(location, (GLfloat)uniform->value.f64[0], (GLfloat)uniform->value.f64[1]); break;
        case TL_BUFFER_DOUBLE3: glUniform3f(location, (GLfloat)uniform->value.f64[0], (GLfloat)uniform->value.f64[1], (GLfloat)uniform->value.f64[2]); break;
        case TL_BUFFER_DOUBLE4: glUniform4f(location, (GLfloat)uniform->value.f64[0], (GLfloat)uniform->value.f64[1], (GLfloat)uniform->value.f64[2], (GLfloat)uniform->value.f64[3]); break;

        default:
            TLERROR("Unsupported uniform type: %d", uniform->type)
            TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(true)
}

// ---------------------------------
// Shader API Internal Functions (run on graphics thread)
// ---------------------------------

// Helper structure for shader creation arguments
typedef struct {
    TLAllocator* allocator;
    u32 count;
    TLShaderSource* sources;
} TLShaderCreateArgs;

/**
 * @brief Internal shader creation (must be called on graphics thread)
 */
static void* tl_shader_create_internal(void** args) {
    TL_PROFILER_PUSH

    TLShaderCreateArgs* create_args = (TLShaderCreateArgs*)args[0];
    TLAllocator* allocator = create_args->allocator;
    const u32 count = create_args->count;
    const TLShaderSource* sources = create_args->sources;

    if (count == 0) {
        TLERROR("Cannot create shader with zero sources")
        TL_PROFILER_POP_WITH(NULL)
    }

    // Allocate shader object
    TLShader* shader = (TLShader*)tl_memory_alloc(allocator, TL_MEMORY_GRAPHICS, sizeof(TLShader));
    if (!shader) {
        TLERROR("Failed to allocate shader object")
        TL_PROFILER_POP_WITH(NULL)
    }

    // Temporary arrays for shader compilation
    u32* shader_ids = (u32*)tl_memory_alloc(m_allocator, TL_MEMORY_BLOCK, sizeof(u32) * count);
    if (!shader_ids) {
        TLERROR("Failed to allocate temporary shader ID array")
        tl_memory_free(allocator, shader);
        TL_PROFILER_POP_WITH(NULL)
    }

    u32 compiled_count = 0;
    for (u32 i = 0; i < count; i++) {
        const TLShaderSource source = sources[i];

        // Read shader source from file
        const char* path = tl_string_cstr(source.path);
        char* code = tl_shader_read_file(path);
        if (!code) {
            TLERROR("Failed to read shader source from: %s", path)
            // Cleanup previously compiled shaders
            for (u32 j = 0; j < compiled_count; j++) {
                glDeleteShader(shader_ids[j]);
            }
            tl_memory_free(m_allocator, shader_ids);
            tl_memory_free(allocator, shader);
            TL_PROFILER_POP_WITH(NULL)
        }

        // Compile shader
        u32 shader_id = tl_shader_compile(source.type, code);
        tl_memory_free(m_allocator, code);

        if (shader_id == 0) {
            TLERROR("Failed to compile shader from: %s", path)
            // Cleanup previously compiled shaders
            for (u32 j = 0; j < compiled_count; j++) {
                glDeleteShader(shader_ids[j]);
            }
            tl_memory_free(m_allocator, shader_ids);
            tl_memory_free(allocator, shader);
            TL_PROFILER_POP_WITH(NULL)
        }

        shader_ids[compiled_count++] = shader_id;
        TLDEBUG("Compiled shader stage %u/%u from: %s", i + 1, count, path)
    }

    // Link shader program
    u32 program_id = tl_shader_link(shader_ids, compiled_count);
    tl_memory_free(m_allocator, shader_ids);

    if (program_id == 0) {
        TLERROR("Failed to link shader program")
        tl_memory_free(allocator, shader);
        TL_PROFILER_POP_WITH(NULL)
    }

    shader->handle = program_id;
    TLINFO("Shader program created successfully (ID: %u)", program_id)

    TL_PROFILER_POP_WITH(shader)
}

/**
 * @brief Internal shader destruction (must be called on graphics thread)
 */
static void* tl_shader_destroy_internal(void** args) {
    TL_PROFILER_PUSH

    TLShader* shader = (TLShader*)args[0];

    if (!shader) {
        TLWARN("Attempting to destroy NULL shader")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (shader->handle != 0) {
        glDeleteProgram((GLuint)shader->handle);
        TLDEBUG("Deleted shader program (ID: %llu)", shader->handle)
    }

    tl_memory_free(m_allocator, shader);
    TL_PROFILER_POP_WITH(NULL)
}

/**
 * @brief Internal shader bind (must be called on graphics thread)
 */
static void* tl_shader_bind_internal(void** args) {
    TL_PROFILER_PUSH

    TLShader* shader = (TLShader*)args[0];

    if (!shader) {
        TLWARN("Attempting to bind NULL shader")
        TL_PROFILER_POP_WITH(NULL)
    }

    glUseProgram((GLuint)shader->handle);

    TL_PROFILER_POP_WITH(NULL)
}

// Helper structure for shader uniform submission
typedef struct {
    TLShader* shader;
    u8 count;
    const char** names;
    TLShaderUniform* uniforms;
} TLShaderSubmitArgs;

/**
 * @brief Internal shader uniform submission (must be called on graphics thread)
 */
static void* tl_shader_submit_internal(void** args) {
    TL_PROFILER_PUSH

    TLShaderSubmitArgs* submit_args = (TLShaderSubmitArgs*)args[0];
    TLShader* shader = submit_args->shader;
    u8 count = submit_args->count;
    const char** names = submit_args->names;
    TLShaderUniform* uniforms = submit_args->uniforms;

    if (!shader) {
        TLERROR("Cannot submit uniforms to NULL shader")
        TL_PROFILER_POP_WITH((void*)(uintptr_t)false)
    }

    if (count == 0) {
        TLWARN("Submitting zero uniforms to shader")
        TL_PROFILER_POP_WITH((void*)(uintptr_t)true)
    }

    // Ensure shader is bound
    glUseProgram((GLuint)shader->handle);

    b8 success = true;
    for (u8 i = 0; i < count; i++) {
        if (!tl_shader_upload_uniform((u32)shader->handle, names[i], &uniforms[i])) {
            TLWARN("Failed to upload uniform '%s' (%u/%u)", names[i], i + 1, count)
            success = false;
        }
    }

    if (success) {
        TLDEBUG("Successfully uploaded %u uniforms to shader", count)
    }

    TL_PROFILER_POP_WITH((void*)(uintptr_t)success)
}

// ---------------------------------
// Shader API Public Functions
// ---------------------------------

TLShader* _tl_shader_create(TLAllocator* allocator, const u32 count, ...) {
    TL_PROFILER_PUSH_WITH("count=%u", count)

    // Collect variadic arguments into array
    TLShaderSource* sources = (TLShaderSource*)tl_memory_alloc(m_allocator, TL_MEMORY_BLOCK, sizeof(TLShaderSource) * count);

    va_list args;
    va_start(args, count);
    for (u32 i = 0; i < count; i++) {
        sources[i] = va_arg(args, TLShaderSource);
    }
    va_end(args);

    // Prepare arguments for graphics thread
    TLShaderCreateArgs create_args = {
        .allocator = allocator,
        .count = count,
        .sources = sources
    };

    TLShader* result = NULL;

    // Check if we're on graphics thread
    if (tl_graphics_is_thread()) {
        // Execute directly
        TLDEBUG("Executing shader creation directly on graphics thread")
        void* arg_ptr = &create_args;
        result = (TLShader*)tl_shader_create_internal(&arg_ptr);
    } else {
        // Submit to graphics queue
        TLDEBUG("Submitting shader creation to graphics queue")
        result = (TLShader*)tl_graphics_submit_sync_args(tl_shader_create_internal, &create_args);
    }

    // Cleanup sources array
    tl_memory_free(m_allocator, sources);

    TL_PROFILER_POP_WITH(result)
}

void tl_shader_destroy(TLShader* shader) {
    TL_PROFILER_PUSH_WITH("shader=0x%p", shader)

    if (!shader) {
        TLWARN("Attempting to destroy NULL shader")
        TL_PROFILER_POP
        return;
    }

    // Check if we're on graphics thread
    if (tl_graphics_is_thread()) {
        // Execute directly
        TLDEBUG("Executing shader destruction directly on graphics thread")
        void* arg_ptr = shader;
        tl_shader_destroy_internal(&arg_ptr);
    } else {
        // Submit to graphics queue
        TLDEBUG("Submitting shader destruction to graphics queue")
        tl_graphics_submit_sync_args(tl_shader_destroy_internal, shader);
    }

    TL_PROFILER_POP
}

void tl_shader_bind(TLShader* shader) {
    TL_PROFILER_PUSH_WITH("shader=0x%p", shader)

    if (!shader) {
        TLWARN("Attempting to bind NULL shader")
        TL_PROFILER_POP
        return;
    }

    // Check if we're on graphics thread
    if (tl_graphics_is_thread()) {
        // Execute directly
        TLDEBUG("Executing shader bind directly on graphics thread")
        void* arg_ptr = shader;
        tl_shader_bind_internal(&arg_ptr);
    } else {
        // Submit to graphics queue (async since we don't need result)
        TLDEBUG("Submitting shader bind to graphics queue")
        tl_graphics_submit_sync_args(tl_shader_bind_internal, shader);
    }

    TL_PROFILER_POP
}

b8 _tl_shader_submit(TLShader* shader, const u8 count, ...) {
    TL_PROFILER_PUSH_WITH("shader=0x%p, count=%u", shader, count)

    if (!shader) {
        TLERROR("Cannot submit uniforms to NULL shader")
        TL_PROFILER_POP_WITH(false)
    }

    if (count == 0) {
        TLWARN("Submitting zero uniforms to shader")
        TL_PROFILER_POP_WITH(true)
    }

    // Collect variadic arguments into arrays
    const char** names = (const char**)tl_memory_alloc(m_allocator, TL_MEMORY_BLOCK, sizeof(const char*) * count);
    TLShaderUniform* uniforms = (TLShaderUniform*)tl_memory_alloc(m_allocator, TL_MEMORY_BLOCK, sizeof(TLShaderUniform) * count);

    if (!names || !uniforms) {
        TLERROR("Failed to allocate uniform arrays")
        if (names) tl_memory_free(m_allocator, names);
        if (uniforms) tl_memory_free(m_allocator, uniforms);
        TL_PROFILER_POP_WITH(false)
    }

    va_list args;
    va_start(args, count);
    for (u8 i = 0; i < count; i++) {
        names[i] = va_arg(args, const char*);
        uniforms[i] = va_arg(args, TLShaderUniform);
    }
    va_end(args);

    // Prepare arguments for graphics thread
    TLShaderSubmitArgs submit_args = {
        .shader = shader,
        .count = count,
        .names = names,
        .uniforms = uniforms
    };

    b8 result = false;

    // Check if we're on graphics thread
    if (tl_graphics_is_thread()) {
        // Execute directly
        TLDEBUG("Executing shader uniform submission directly on graphics thread")
        void* arg_ptr = &submit_args;
        result = (b8)(uintptr_t)tl_shader_submit_internal(&arg_ptr);
    } else {
        // Submit to graphics queue
        TLDEBUG("Submitting shader uniform submission to graphics queue")
        result = (b8)(uintptr_t)tl_graphics_submit_sync_args(tl_shader_submit_internal, &submit_args);
    }

    // Cleanup arrays
    tl_memory_free(m_allocator, names);
    tl_memory_free(m_allocator, uniforms);

    TL_PROFILER_POP_WITH(result)
}

#endif
