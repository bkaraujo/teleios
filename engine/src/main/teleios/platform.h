/**
 * @file platform.h
 * @brief Platform abstraction layer for cross-platform OS operations
 *
 * Provides a unified interface for platform-specific operations including:
 * - Platform initialization and cleanup
 * - High-resolution timing (microsecond precision)
 * - Date/time retrieval
 * - Filesystem path separators
 *
 * The platform layer uses compile-time dispatch to select the appropriate
 * implementation (Windows, Linux, macOS) based on platform macros defined
 * in defines.h.
 *
 * @section implementations Platform Implementations
 * - **Windows**: Uses QueryPerformanceCounter for timing (platform_windows.inl)
 * - **Linux**: Uses clock_gettime() for timing (platform_linux.inl)
 * - **GLFW**: Window management across all platforms (platform_glfw.inl)
 *
 * @section timing Timing Functions
 * The platform layer provides three timing functions:
 * - tl_time_clock(): Get current date and time
 * - tl_time_epoch_millis(): Milliseconds since Unix epoch
 * - tl_time_epoch_micros(): Microseconds since Unix epoch
 *
 * @section initialization Initialization
 * The platform must be initialized before any other engine systems:
 * @code
 * if (!tl_platform_initialize()) {
 *     TLFATAL("Platform initialization failed");
 * }
 *
 * // Use platform functions...
 *
 * tl_platform_terminate();
 * @endcode
 *
 * @note Platform initialization also initializes:
 * - Memory system (tl_memory_initialize)
 * - GLFW library (glfwInit)
 * - Window creation (tl_window_create)
 * - Graphics system (tl_graphics_initialize)
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

#ifndef __TELEIOS_PLATFORM__
#define __TELEIOS_PLATFORM__

#include "teleios/defines.h"

/**
 * @brief Initialize platform layer and all dependent subsystems
 *
 * This function performs platform-specific initialization including:
 * - High-resolution timer calibration (Windows: QueryPerformanceFrequency)
 * - Memory system initialization
 * - GLFW library initialization
 * - Window creation (initially hidden)
 * - Graphics worker thread creation
 *
 * Must be called before any other engine functions.
 *
 * @return true on success, false on failure
 *
 * @note On failure, cleanup of already-initialized subsystems is automatic
 *
 * @see tl_platform_terminate
 * @see tl_memory_initialize
 * @see tl_graphics_initialize
 */
b8 tl_platform_initialize(void);

/**
 * @brief Terminate platform layer and cleanup all subsystems
 *
 * Cleanup order (reverse of initialization):
 * 1. Graphics system (join worker thread)
 * 2. Window destruction
 * 3. GLFW termination
 * 4. Platform-specific cleanup
 * 5. Memory system (reports leaks in DYNAMIC allocators)
 *
 * @return true on success, false on failure
 *
 * @note Should be called after all other engine systems are terminated
 * @warning Calling engine functions after termination is undefined behavior
 *
 * @see tl_platform_initialize
 */
b8 tl_platform_terminate(void);

#endif
