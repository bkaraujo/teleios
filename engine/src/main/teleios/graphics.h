#ifndef __TELEIOS_GRAPHICS__
#define __TELEIOS_GRAPHICS__

#include "teleios/defines.h"

b8 tl_graphics_initialize(void);
b8 tl_graphics_terminate(void);

/**
 * @brief Submits synchronous work without parameters to the graphics thread
 *
 * Blocks the calling thread until the work is completed by the graphics thread.
 * Safe to use stack-allocated data in the caller's scope since execution is synchronous.
 *
 * @param func Function to execute on the graphics thread
 *
 * Example:
 * @code
 * void setup_shaders(void) {
 *     // OpenGL shader initialization
 * }
 *
 * tl_graphics_submit_sync(setup_shaders);  // Blocks until complete
 * @endcode
 */
TL_API void tl_graphics_submit_sync(void (*func)(void));

/**
 * @brief Submits synchronous work with parameters to the graphics thread
 *
 * Blocks the calling thread until the work is completed by the graphics thread.
 * Safe to use stack-allocated data in args since execution is synchronous.
 *
 * @param func Function to execute on the graphics thread
 * @param args Arguments to pass to the function (can be stack-allocated)
 *
 * Example:
 * @code
 * void load_texture(void* args) {
 *     TextureData* data = (TextureData*)args;
 *     // OpenGL texture loading
 * }
 *
 * TextureData data = {.width = 512, .height = 512};
 * tl_graphics_submit_sync_args(load_texture, &data);  // Blocks until complete
 * @endcode
 */
TL_API void tl_graphics_submit_sync_args(void (*func)(void*), void* args);

/**
 * @brief Submits asynchronous work without parameters to the graphics thread
 *
 * Returns immediately without waiting for execution. Work is queued and will
 * be executed when the graphics thread processes it.
 *
 * @param func Function to execute on the graphics thread
 *
 * Example:
 * @code
 * void clear_framebuffer(void) {
 *     glClear(GL_COLOR_BUFFER_BIT);
 * }
 *
 * tl_graphics_submit_async(clear_framebuffer);  // Returns immediately
 * @endcode
 */
TL_API void tl_graphics_submit_async(void (*func)(void));

/**
 * @brief Submits asynchronous work with parameters to the graphics thread
 *
 * Returns immediately without waiting for execution. Work is queued and will
 * be executed when the graphics thread processes it.
 *
 * IMPORTANT: Args must remain valid until the work executes. Consider using
 * heap-allocated memory and freeing it inside the work function, or ensure
 * the data has appropriate lifetime (e.g., global/static storage).
 *
 * @param func Function to execute on the graphics thread
 * @param args Arguments to pass to the function (must have appropriate lifetime)
 *
 * Example:
 * @code
 * void update_buffer(void* args) {
 *     BufferData* data = (BufferData*)args;
 *     // OpenGL buffer update
 *     free(data);  // Clean up heap allocation
 * }
 *
 * BufferData* data = malloc(sizeof(BufferData));
 * data->size = 1024;
 * tl_graphics_submit_async_args(update_buffer, data);  // Returns immediately
 * @endcode
 */
TL_API void tl_graphics_submit_async_args(void (*func)(void*), void* args);

// ---------------------------------
// Module Lifecycle
// ---------------------------------

/**
 * @brief Initializes the graphics thread system
 *
 * Creates the work queue, worker thread, and subscribes to shutdown events.
 * Transfers OpenGL context ownership from main thread to worker thread.
 *
 * MUST be called after graphics system initialization and before submitting work.
 *
 * @return true on success, false on failure
 */
TL_API b8 tl_graphics_thread_initialize(void);

/**
 * @brief Terminates the graphics thread system
 *
 * Signals shutdown, waits for worker thread to finish processing pending work,
 * and cleans up all resources.
 *
 * @return true on success, false on failure
 */
TL_API b8 tl_graphics_thread_terminate(void);

#endif