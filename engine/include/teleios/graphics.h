#ifndef __TELEIOS_GRAPHICS__
#define __TELEIOS_GRAPHICS__

#include "teleios/defines.h"

/**
 * @brief Initialize graphics subsystem and start worker thread
 *
 * Initializes the graphics system by:
 * 1. Creating the dedicated graphics worker thread
 * 2. Transferring OpenGL context to the worker thread
 * 3. Initializing GLAD for OpenGL function loading
 * 4. Logging OpenGL and graphics library versions
 *
 * Must be called after window creation. Typically called by tl_platform_initialize().
 *
 * @return true on success, false on failure
 *
 * @note The worker thread runs continuously until tl_graphics_terminate()
 * @note GLAD is initialized internally - application code should not call gladLoadGLLoader()
 * @note OpenGL context becomes inaccessible from the main thread after this call
 *
 * @see tl_graphics_terminate
 * @see tl_graphics_submit_sync
 * @see tl_graphics_submit_async
 *
 * @code
 * // Initialize graphics after window creation
 * if (!tl_graphics_initialize()) {
 *     TLFATAL("Failed to initialize graphics");
 * }
 *
 * TLINFO("Graphics system ready for work submission");
 * @endcode
 */
b8 tl_graphics_initialize(void);

/**
 * @brief Terminate graphics subsystem and wait for worker thread
 *
 * Shuts down the graphics system by:
 * 1. Signaling the worker thread to stop accepting new work
 * 2. Waiting for all pending work to complete
 * 3. Joining the worker thread
 * 4. Releasing the OpenGL context
 *
 * Ensures all submitted work is processed before termination. Should be called
 * before window destruction. Typically called by tl_platform_terminate().
 *
 * @return true on success, false on failure
 *
 * @note Blocks until all pending work completes (may take time)
 * @note No new work can be submitted after this call
 * @note OpenGL becomes unavailable after this call
 *
 * @see tl_graphics_initialize
 * @see tl_graphics_submit_sync
 * @see tl_graphics_submit_async
 *
 * @code
 * // Shutdown graphics system
 * TLINFO("Shutting down graphics...");
 * if (!tl_graphics_terminate()) {
 *     TLERROR("Graphics termination failed");
 *     return false;
 * }
 * TLINFO("Graphics system shut down");
 * @endcode
 */
b8 tl_graphics_terminate(void);

// ---------------------------------------------------
// Graphics Task API
// ---------------------------------------------------

/**
 * @brief Submit graphics work synchronously (no parameters)
 *
 * Submits a graphics job to the worker thread and blocks until completion.
 * Returns the value returned by the job function.
 *
 * Use for setup operations that need to complete before continuing:
 * - Shader compilation
 * - Texture loading
 * - Buffer creation
 * - Other GPU resource initialization
 *
 * @param func Function pointer to execute (void* (*)(void))
 * @return Value returned by the job function
 *
 * @note Blocks until job completes
 * @note Safe to use stack-allocated data (deallocation after return is safe)
 * @note Function is called on the graphics thread
 *
 * @see tl_graphics_submit_async
 * @see tl_graphics_submit_sync_args
 *
 * @code
 * static void* initialize_opengl(void) {
 *     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
 *     glEnable(GL_DEPTH_TEST);
 *     glEnable(GL_CULL_FACE);
 *     return NULL;
 * }
 *
 * // Wait for OpenGL initialization
 * tl_graphics_submit_sync(initialize_opengl);
 * TLINFO("OpenGL initialized");
 * @endcode
 */
void* tl_graphics_submit_sync(void* (*func)(void));

/**
 * @brief Submit graphics work asynchronously (no parameters)
 *
 * Submits a graphics job to the worker thread and returns immediately.
 * Does not wait for job completion.
 *
 * Use for recurring operations that don't need immediate results:
 * - Frame rendering
 * - Animation updates
 * - Particle system updates
 *
 * @param func Function pointer to execute (void* (*)(void))
 * @return Value returned by the job function (if any)
 *
 * @note Returns immediately; job executes asynchronously
 * @note Function is called on the graphics thread
 * @note Do not rely on execution time - job may be queued
 * @note Maximum queue capacity is 256 jobs; submission blocks if full
 *
 * @see tl_graphics_submit_sync
 * @see tl_graphics_submit_async_args
 *
 * @code
 * static void* render_frame(void) {
 *     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 *
 *     // Render scene
 *     render_scene();
 *
 *     glfwSwapBuffers(tl_window_handler());
 *     return NULL;
 * }
 *
 * while (running) {
 *     update_game(delta_time);
 *     tl_graphics_submit_async(render_frame);  // Non-blocking
 * }
 * @endcode
 */
void* tl_graphics_submit_async(void* (*func)(void));

/**
 * @brief Submit graphics work synchronously with variadic arguments
 *
 * Submits a graphics job with multiple arguments to the worker thread and blocks
 * until completion. Arguments are automatically packed into an array and passed
 * to the function. Returns the value returned by the job function.
 *
 * Use for setup operations with specific parameters:
 * - Loading textures with filenames
 * - Creating resources with configuration
 * - Parameterized initialization
 *
 * @param func Function pointer to execute (void* (*)(void**))
 * @param ... Variable arguments to pass to the function (max 16 args)
 * @return Value returned by the job function
 *
 * @note Blocks until job completes
 * @note Safe to use stack-allocated arguments (function blocks until return)
 * @note Function is called on the graphics thread
 * @note Arguments are passed as void** array to the function
 * @note Argument validity is guaranteed until function returns
 * @note Maximum 16 arguments supported
 *
 * @see tl_graphics_submit_sync
 * @see tl_graphics_submit_async_args
 *
 * @code
 * static void* load_texture_job(void** args) {
 *     const char* filename = (const char*)args[0];
 *     GLuint* out_texture = (GLuint*)args[1];
 *
 *     int width, height, channels;
 *     unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
 *
 *     glGenTextures(1, out_texture);
 *     glBindTexture(GL_TEXTURE_2D, *out_texture);
 *     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
 *
 *     stbi_image_free(data);
 *     return NULL;
 * }
 *
 * // Load texture with varargs
 * GLuint wall_texture;
 * tl_graphics_submit_sync_args(load_texture_job, "wall.png", &wall_texture);
 * // wall_texture now contains the loaded texture handle
 * @endcode
 */
#define tl_graphics_submit_sync_args(func, ...) _tl_graphics_submit_sync_args(func, TL_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
void* _tl_graphics_submit_sync_args(void* (*func)(void**), u32 count, ...);

/**
 * @brief Submit graphics work asynchronously with variadic arguments
 *
 * Submits a graphics job with multiple arguments to the worker thread and returns
 * immediately. Arguments are automatically packed into a heap-allocated array.
 * Does not wait for job completion.
 *
 * Use for recurring operations with specific parameters:
 * - Rendering with configuration
 * - Animation updates with parameters
 * - Dynamic resource binding
 *
 * @param func Function pointer to execute (void* (*)(void**))
 * @param ... Variable arguments to pass to the function (max 16 args)
 * @return NULL (async jobs don't return values)
 *
 * @note Returns immediately; job executes asynchronously
 * @note Arguments are automatically heap-allocated and freed after job completes
 * @note Passed arguments are COPIED to heap (pointers themselves, not pointed-to data)
 * @note If arguments point to data, ensure that data remains valid or is heap-allocated
 * @note Function is called on the graphics thread
 * @note Maximum 16 arguments supported
 *
 * @warning If you pass pointers to stack data, ensure the data outlives the job
 *          or allocate the data on the heap before passing the pointer
 *
 * @see tl_graphics_submit_async
 * @see tl_graphics_submit_sync_args
 *
 * @code
 * static void* render_mesh_job(void** args) {
 *     GLuint vao = (GLuint)(uintptr_t)args[0];
 *     u32 vertex_count = (u32)(uintptr_t)args[1];
 *
 *     glBindVertexArray(vao);
 *     glDrawArrays(GL_TRIANGLES, 0, vertex_count);
 *
 *     return NULL;
 * }
 *
 * // Arguments are automatically managed
 * tl_graphics_submit_async_args(render_mesh_job, (void*)(uintptr_t)mesh.vao, (void*)(uintptr_t)mesh.vertex_count);
 * @endcode
 */
#define tl_graphics_submit_async_args(func, ...) _tl_graphics_submit_async_args(func, TL_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
void* _tl_graphics_submit_async_args(void* (*func)(void**), u32 count, ...);

// ---------------------------------------------------
// Graphics Shader API
// ---------------------------------------------------

/**
 *  @brief Creates a a new Shared
 *
 *  @param allocator The memory allocator
 *  @param ... Variable arguments to pass to the function (max 16 args)
 *
 *  @note  THe variadic parameters expected are the desired TLShaderSources
 */
#define tl_shader_create(allocator, ...) _tl_shader_create(allocator, TL_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
TLShader* _tl_shader_create(TLAllocator* allocator, u32 count, ...);

/**
 * @brief Destroy the shader both in the CPU and GPU
 * @param shader
 */
void tl_shader_destroy(TLShader* shader);

/**
 * @brief Bind the shader for the next draw call
 * @param shader The shader that will be destroyed
 */
void tl_shader_bind(TLShader* shader);

/**
 * @brief Upload to the GPU some arbitrary shader data
 *
 * @code
 * TLUniform u1 = {
 *  .type = TL_BUFFER_FLOAT2;
 *  .value.f32[0] = 0.27;
 *  .value.f32[1] = 0.18;
 * }
 *
 * TLUniform u2 = {
 *  .type = TL_BUFFER_FLOAT1;
 *  .value.f32[0] = 0.27;
 * }
 *
 * if (!tl_shader_submit(shader, u1, u2)) {
 *  TLFATAL("Failed to submit uniform")
 * }
 *
 * @param shader The shader that will receive the data
 */
#define tl_shader_submit(shader, ...) _tl_shader_submit(shader, TL_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
b8 _tl_shader_submit(TLShader* shader, u8 count, ...);

// ---------------------------------------------------
// Graphics Texture API
// ---------------------------------------------------

// ---------------------------------------------------
// Graphics FrameBuffer API
// ---------------------------------------------------

// ---------------------------------------------------
// Command Buffer API (Triple Buffering)
// ---------------------------------------------------

/**
 * @brief Begin recording commands to a command buffer
 *
 * Acquires a free command buffer from the triple buffer pool and begins
 * recording. All subsequent graphics submissions will be recorded to this
 * buffer until tl_cmdbuffer_end() is called.
 *
 * This function may block if no buffers are available (all 3 are pending
 * or executing). In practice with triple buffering, a buffer should always
 * be available.
 *
 * @note Must be called from main thread before any graphics work submission
 * @note Call tl_cmdbuffer_end() when done recording frame commands
 *
 * @see tl_cmdbuffer_end
 * @see tl_cmdbuffer_sync
 */
void tl_cmdbuffer_begin(void);

/**
 * @brief End recording and submit command buffer for execution
 *
 * Finalizes the current command buffer and submits it to the graphics thread
 * for execution. Returns immediately without waiting for execution to complete.
 *
 * This enables pipelining: main thread can start recording frame N+1 while
 * graphics thread executes frame N.
 *
 * @note Must be called after tl_cmdbuffer_begin()
 * @note Does not block - returns immediately
 * @note Graphics thread will process the buffer asynchronously
 *
 * @see tl_cmdbuffer_begin
 * @see tl_cmdbuffer_sync
 */
void tl_cmdbuffer_end(void);

/**
 * @brief Wait for all pending command buffers to complete
 *
 * Blocks until all submitted command buffers have finished executing.
 * Use sparingly - typically only needed at frame boundaries when you need
 * to ensure the GPU has caught up.
 *
 * @note Blocks until graphics thread finishes all pending buffers
 * @note Call at start of frame to ensure previous frame completed
 *
 * @see tl_cmdbuffer_begin
 * @see tl_cmdbuffer_end
 */
void tl_cmdbuffer_sync(void);

#endif
