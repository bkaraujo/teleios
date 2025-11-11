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
