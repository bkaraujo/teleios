/**
 * @file graphics.h
 * @brief Graphics subsystem with dedicated rendering thread and OpenGL support
 *
 * This module provides a thread-safe graphics system with a dedicated worker thread
 * for all OpenGL operations. It implements a producer-consumer pattern where the
 * main thread submits graphics work and the graphics thread processes it.
 *
 * @section threading_model Threading Architecture
 *
 * The graphics system uses a dedicated worker thread to handle all OpenGL operations:
 *
 * ```
 * Main Thread                    Graphics Thread
 * -----------                    ---------------
 * • Game logic                   • OpenGL context ownership
 * • Physics                      • GLAD initialization
 * • glfwPollEvents()             • Process work queue
 * • Submit render commands       • Block when queue empty
 *   via submit functions  ---->  • Process until shutdown
 * ```
 *
 * **Key architectural points:**
 * - Main thread: Never acquires OpenGL context
 * - Graphics thread: Exclusively owns the OpenGL context
 * - GLAD initialization: Performed on graphics thread
 * - Work queue: Thread-safe with mutex + condition variable
 * - Shutdown: Graceful - processes pending work before exit
 *
 * @section work_submission Work Submission
 *
 * Four submission functions provide different synchronization modes:
 *
 * **Synchronous submissions** (block until completion):
 * - `tl_graphics_submit_sync(func)` - No parameters
 * - `tl_graphics_submit_sync_args(func, args)` - With parameters
 *
 * **Asynchronous submissions** (return immediately):
 * - `tl_graphics_submit_async(func)` - No parameters
 * - `tl_graphics_submit_async_args(func, args)` - With parameters
 *
 * @section memory_considerations Memory and Lifetime
 *
 * **For synchronous jobs:**
 * - Can use stack-allocated arguments (safe to deallocate after return)
 * - Function blocks until completion
 * - Arguments can be freed immediately after call
 *
 * **For asynchronous jobs:**
 * - Arguments must remain valid until function executes
 * - Use heap-allocated memory or static data
 * - Function returns immediately; execution is deferred
 *
 * @section performance Performance Considerations
 *
 * **Queue capacity:** 256 jobs maximum
 * - Exceeding capacity blocks submission
 * - Synchronous jobs automatically wait for capacity
 * - Asynchronous jobs should check return values
 *
 * **Memory allocation:**
 * - Graphics thread has 1MB dedicated allocator (tag: TL_MEMORY_GRAPHICS)
 * - Allocate temporary graphics resources from this allocator
 * - Clear resources before shutdown
 *
 * @section usage Usage Examples
 *
 * **Initialize graphics system:**
 * @code
 * #include "teleios/graphics.h"
 *
 * if (!tl_graphics_initialize()) {
 *     TLFATAL("Graphics system initialization failed");
 * }
 *
 * // Graphics thread is now running
 * @endcode
 *
 * **Submit synchronous rendering command:**
 * @code
 * // Define rendering function
 * static void* setup_shader(void) {
 *     GLuint program = glCreateProgram();
 *     // ... shader compilation ...
 *     return (void*)(uintptr_t)program;
 * }
 *
 * // Submit and wait for completion
 * void* result = tl_graphics_submit_sync(setup_shader);
 * GLuint shader_program = (GLuint)(uintptr_t)result;
 * @endcode
 *
 * **Submit asynchronous rendering command:**
 * @code
 * static void* render_frame(void) {
 *     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 *     glDrawArrays(GL_TRIANGLES, 0, 3);
 *     glfwSwapBuffers(tl_window_handler());
 *     return NULL;
 * }
 *
 * // In game loop
 * while (running) {
 *     // Update game state
 *     update_game(delta_time);
 *
 *     // Submit render command (non-blocking)
 *     tl_graphics_submit_async(render_frame);
 * }
 * @endcode
 *
 * **Submit with parameters (synchronous):**
 * @code
 * struct LoadTextureArgs {
 *     const char* filename;
 *     GLuint* out_texture;
 * };
 *
 * static void* load_texture_job(void* args) {
 *     LoadTextureArgs* arg = (LoadTextureArgs*)args;
 *     // Load texture using stb_image
 *     *(arg->out_texture) = glHandle;
 *     return NULL;
 * }
 *
 * // Stack-allocate args (safe because synchronous)
 * GLuint texture;
 * LoadTextureArgs args = { "wall.png", &texture };
 * tl_graphics_submit_sync_args(load_texture_job, &args);
 *
 * // texture is now loaded
 * @endcode
 *
 * **Submit with parameters (asynchronous):**
 * @code
 * struct RenderMeshArgs {
 *     GLuint vao;
 *     u32 vertex_count;
 * };
 *
 * static void* render_mesh_job(void* args) {
 *     RenderMeshArgs* arg = (RenderMeshArgs*)args;
 *     glBindVertexArray(arg->vao);
 *     glDrawArrays(GL_TRIANGLES, 0, arg->vertex_count);
 *     free(arg);  // Async jobs must free their args
 *     return NULL;
 * }
 *
 * // Heap-allocate args (required for async)
 * RenderMeshArgs* args = malloc(sizeof(RenderMeshArgs));
 * args->vao = mesh_vao;
 * args->vertex_count = 1000;
 *
 * tl_graphics_submit_async_args(render_mesh_job, args);
 * // Args are freed inside the job function
 * @endcode
 *
 * **Shutdown graphics system:**
 * @code
 * // Terminate processes remaining jobs and joins thread
 * if (!tl_graphics_terminate()) {
 *     TLERROR("Graphics termination failed");
 * }
 *
 * // All graphics resources cleaned up
 * // OpenGL context released
 * @endcode
 *
 * @note GLAD initialization is performed automatically during graphics initialization.
 *       Do not call gladLoadGLLoader() in application code.
 *
 * @note All OpenGL calls MUST go through graphics submission. Calling OpenGL
 *       functions from the main thread causes undefined behavior.
 *
 * @note glfwPollEvents() is called from the main thread (not graphics thread).
 *       This is by GLFW design - input/window events must be polled from the
 *       main thread.
 *
 * @note The graphics thread owns the OpenGL context exclusively. Never transfer
 *       the context back to the main thread.
 *
 * @see window.h - Window queries (use in graphics jobs)
 * @see memory.h - Graphics memory allocator (TL_MEMORY_GRAPHICS)
 * @see platform.h - Platform initialization that creates graphics system
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

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
 * @brief Submit graphics work synchronously with parameters
 *
 * Submits a graphics job with arbitrary data to the worker thread and blocks
 * until completion. Returns the value returned by the job function.
 *
 * Use for setup operations with specific parameters:
 * - Loading textures with filenames
 * - Creating resources with configuration
 * - Parameterized initialization
 *
 * @param func Function pointer to execute (void* (*)(void*))
 * @param args Pointer to argument data (can be stack or heap allocated)
 * @return Value returned by the job function
 *
 * @note Blocks until job completes
 * @note Safe to use stack-allocated arguments (function blocks until return)
 * @note Function is called on the graphics thread
 * @note Argument validity is guaranteed until function returns
 *
 * @see tl_graphics_submit_sync
 * @see tl_graphics_submit_async_args
 *
 * @code
 * struct TextureLoadArgs {
 *     const char* filename;
 *     GLuint* out_texture;
 * };
 *
 * static void* load_texture_job(void* args) {
 *     TextureLoadArgs* arg = (TextureLoadArgs*)args;
 *     int width, height, channels;
 *     unsigned char* data = stbi_load(arg->filename, &width, &height, &channels, 4);
 *
 *     glGenTextures(1, arg->out_texture);
 *     glBindTexture(GL_TEXTURE_2D, *arg->out_texture);
 *     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
 *
 *     stbi_image_free(data);
 *     return NULL;
 * }
 *
 * // Load texture with parameters
 * GLuint wall_texture;
 * TextureLoadArgs args = { "wall.png", &wall_texture };
 * tl_graphics_submit_sync_args(load_texture_job, &args);
 *
 * // wall_texture now contains the loaded texture handle
 * @endcode
 */
void* tl_graphics_submit_sync_args(void* (*func)(void*), void* args);

/**
 * @brief Submit graphics work asynchronously with parameters
 *
 * Submits a graphics job with arbitrary data to the worker thread and returns
 * immediately. Does not wait for job completion.
 *
 * Use for recurring operations with specific parameters:
 * - Rendering with configuration
 * - Animation updates with parameters
 * - Dynamic resource binding
 *
 * @param func Function pointer to execute (void* (*)(void*))
 * @param args Pointer to argument data (should be heap-allocated)
 * @return Value returned by the job function (if any)
 *
 * @note Returns immediately; job executes asynchronously
 * @note Arguments MUST remain valid until job execution completes
 * @note Use heap-allocated memory for arguments
 * @note Job function should free arguments when done
 * @note Function is called on the graphics thread
 *
 * @warning Arguments must be heap-allocated or static. Stack-allocated
 *          arguments will be invalid when job executes.
 *
 * @see tl_graphics_submit_async
 * @see tl_graphics_submit_sync_args
 *
 * @code
 * struct RenderMeshArgs {
 *     GLuint vao;
 *     u32 vertex_count;
 * };
 *
 * static void* render_mesh_job(void* args) {
 *     RenderMeshArgs* arg = (RenderMeshArgs*)args;
 *
 *     glBindVertexArray(arg->vao);
 *     glDrawArrays(GL_TRIANGLES, 0, arg->vertex_count);
 *
 *     free(arg);  // Job must free args
 *     return NULL;
 * }
 *
 * // Heap-allocate arguments for async job
 * RenderMeshArgs* args = malloc(sizeof(RenderMeshArgs));
 * args->vao = mesh.vao;
 * args->vertex_count = mesh.vertex_count;
 *
 * tl_graphics_submit_async_args(render_mesh_job, args);
 * // Job will free args when it executes
 * @endcode
 */
void* tl_graphics_submit_async_args(void* (*func)(void*), void* args);

#endif
