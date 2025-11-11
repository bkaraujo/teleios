/**
 * @file window.h
 * @brief Window management and query utilities
 *
 * This module provides access to the application window created by the platform layer.
 * The window is created during tl_platform_initialize() with the following defaults:
 *
 * - **Size**: 1024x768 pixels
 * - **Initial Visibility**: Hidden (not shown until explicitly made visible)
 * - **OpenGL Context**: Version 4.6 Core Profile
 * - **Position**: Centered on primary monitor
 *
 * The window is created using GLFW and is managed by the platform layer. This module
 * provides query functions to access the window handle and retrieve its properties.
 *
 * @section window_creation Window Creation Process
 *
 * The window is created during platform initialization and destroyed during
 * platform termination. The typical lifecycle is:
 *
 * 1. **Initialization**: tl_platform_initialize() creates the window
 * 2. **Usage**: Query window properties and use the handle
 * 3. **Event Loop**: Application polls window events via glfwPollEvents()
 * 4. **Termination**: tl_platform_terminate() destroys the window
 *
 * @section opengl_context OpenGL Context
 *
 * The window has an active OpenGL 4.6 Core Profile context. However:
 * - The context is initially held by the graphics worker thread
 * - The main application thread should NOT call OpenGL functions directly
 * - All OpenGL work must be submitted through graphics submission functions
 * - See graphics.h for graphics submission API
 *
 * @section window_events Window Events
 *
 * Window events (close, resize, move, focus, minimize, maximize) are automatically
 * captured by GLFW callbacks and submitted to the event system. Subscribe to
 * events using tl_event_subscribe() to handle window events.
 *
 * @section usage Usage Examples
 *
 * **Getting the GLFW window pointer:**
 * @code
 * #include "teleios/window.h"
 *
 * // Get the underlying GLFW window pointer
 * GLFWwindow* window = (GLFWwindow*)tl_window_handler();
 *
 * // Use with GLFW functions (carefully - window management is platform's job)
 * glfwGetWindowSize(window, &width, &height);
 * @endcode
 *
 * **Querying window dimensions:**
 * @code
 * // Get window width and height
 * ivec2s size = tl_window_size();
 *
 * TLINFO("Window size: %d x %d", size.x, size.y);
 *
 * // Use for UI layout or viewport setup
 * if (size.x > 1920 || size.y > 1080) {
 *     TLINFO("High resolution display detected");
 * }
 * @endcode
 *
 * **Querying window position:**
 * @code
 * // Get window position on screen
 * ivec2s pos = tl_window_position();
 *
 * TLINFO("Window position: (%d, %d)", pos.x, pos.y);
 *
 * // Use for multi-monitor setups
 * if (pos.x < 0) {
 *     TLINFO("Window extends to secondary monitor on left");
 * }
 * @endcode
 *
 * **Handling window resize events:**
 * @code
 * // Subscribe to window resize events
 * TLEventStatus on_window_resized(const TLEvent* event) {
 *     i32 new_width = event->i32[0];
 *     i32 new_height = event->i32[1];
 *
 *     // Update UI layout, viewport, etc.
 *     update_viewport(new_width, new_height);
 *
 *     return TL_EVENT_CONSUMED;
 * }
 *
 * // In initialization:
 * tl_event_subscribe(TL_EVENT_WINDOW_RESIZED, on_window_resized);
 * @endcode
 *
 * @section query_types Window Query Data Types
 *
 * Window queries return ivec2s structures (from cglm):
 * - x: Horizontal component (width or x-coordinate)
 * - y: Vertical component (height or y-coordinate)
 *
 * @note Do not cache window size/position - query fresh values when needed
 *       as they may change due to user actions (resize, move, etc.)
 *
 * @note The OpenGL context is owned by the graphics thread, not the main thread.
 *       Do not attempt direct OpenGL calls. Use graphics submission functions.
 *
 * @note The window is managed by the platform layer. Do not attempt to destroy
 *       it or modify its properties directly.
 *
 * @see graphics.h - Graphics submission for OpenGL commands
 * @see event.h - Event subscription for window events
 * @see platform.h - Platform initialization that creates window
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

#ifndef __TELEIOS_WINDOW__
#define __TELEIOS_WINDOW__

#include "teleios/defines.h"

/**
 * @brief Get the GLFW window handle
 *
 * Returns the underlying GLFWwindow pointer for the application window.
 * This pointer can be used with GLFW functions, but window management
 * (creation, destruction, state changes) is handled by the platform layer.
 *
 * The returned pointer is valid only between tl_platform_initialize()
 * and tl_platform_terminate().
 *
 * @return GLFWwindow pointer (void* for header independence)
 *
 * @note The window is created during tl_platform_initialize() with:
 *       - Size: 1024x768
 *       - OpenGL 4.6 Core Profile
 *       - Initially hidden
 *       - Centered on primary monitor
 *
 * @note Do not destroy or significantly modify the window. The platform
 *       layer manages its lifecycle.
 *
 * @note The OpenGL context is managed by the graphics worker thread.
 *       Do not attempt direct OpenGL calls from the main thread.
 *
 * @see tl_window_size - Query window dimensions
 * @see tl_window_position - Query window screen position
 * @see graphics.h - For graphics submission instead of direct OpenGL
 *
 * @code
 * // Get the GLFW window pointer
 * GLFWwindow* window = (GLFWwindow*)tl_window_handler();
 *
 * // Safe GLFW operations (read-only)
 * int width, height;
 * glfwGetWindowSize(window, &width, &height);
 *
 * // Query OpenGL extensions
 * if (glfwExtensionSupported("GL_ARB_compute_shader")) {
 *     TLINFO("Compute shaders supported");
 * }
 * @endcode
 */
void* tl_window_handler();

/**
 * @brief Get current window dimensions
 *
 * Returns the current width and height of the application window in pixels.
 * This value may change if the window is resized by the user or programmatically.
 *
 * @return ivec2s structure with:
 *         - x: Window width in pixels
 *         - y: Window height in pixels
 *
 * @note The returned values reflect the window client area (drawable area),
 *       not including window frame/decorations.
 *
 * @note Window events (TL_EVENT_WINDOW_RESIZED) contain updated dimensions
 *       and are submitted before this function returns a new size.
 *
 * @note Do not cache this value - query fresh when needed as the window
 *       size can change due to user actions.
 *
 * @see tl_window_position - Query window screen position
 * @see tl_window_handler - Get GLFW window pointer
 * @see event.h - Subscribe to TL_EVENT_WINDOW_RESIZED
 *
 * @code
 * // Get window dimensions
 * ivec2s size = tl_window_size();
 *
 * TLINFO("Window: %d x %d", size.x, size.y);
 *
 * // Use for viewport/camera setup
 * f32 aspect_ratio = (f32)size.x / (f32)size.y;
 * setup_camera(aspect_ratio);
 *
 * // Check for ultra-wide displays
 * if (size.x > size.y * 1.5f) {
 *     TLINFO("Ultra-wide display detected");
 * }
 * @endcode
 */
ivec2s tl_window_size();

/**
 * @brief Get current window screen position
 *
 * Returns the window's position on the screen (relative to the top-left corner
 * of the primary monitor). This is useful for multi-monitor setups or saving
 * window positions.
 *
 * @return ivec2s structure with:
 *         - x: Window's left edge x-coordinate in screen pixels
 *         - y: Window's top edge y-coordinate in screen pixels
 *
 * @note The position is measured from the top-left of the monitor that
 *       contains the window's top-left corner.
 *
 * @note On multi-monitor setups, coordinates may be negative if the window
 *       extends to monitors to the left or above the primary display.
 *
 * @note Window events (TL_EVENT_WINDOW_MOVED) contain updated position
 *       and are submitted before this function returns a new position.
 *
 * @note Do not cache this value - query fresh when needed as the window
 *       position can change due to user actions.
 *
 * @see tl_window_size - Query window dimensions
 * @see tl_window_handler - Get GLFW window pointer
 * @see event.h - Subscribe to TL_EVENT_WINDOW_MOVED
 *
 * @code
 * // Get window position
 * ivec2s pos = tl_window_position();
 *
 * TLINFO("Window position: (%d, %d)", pos.x, pos.y);
 *
 * // Detect multi-monitor setup
 * if (pos.x < 0) {
 *     TLINFO("Window extends to secondary monitor on left side");
 * } else if (pos.x > 1920) {
 *     TLINFO("Window is on secondary monitor on right side");
 * }
 *
 * // Save position for restoration on next launch
 * save_window_state(pos.x, pos.y);
 * @endcode
 */
ivec2s tl_window_position();

#endif
