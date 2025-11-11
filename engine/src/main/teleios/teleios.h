/**
 * @file teleios.h
 * @brief Main TELEIOS Engine header - includes all engine subsystems
 *
 * This is the primary include file for the TELEIOS game engine. It aggregates
 * all core engine modules into a single convenient header.
 *
 * Include this file in your application to access all engine functionality:
 * @code
 * #include "teleios/teleios.h"
 * @endcode
 *
 * @section modules Engine Modules
 * The engine is organized into the following subsystems:
 * - **Platform**: OS abstraction layer (timing, filesystem, window management)
 * - **Memory**: Custom allocators (LINEAR, DYNAMIC) with leak detection
 * - **Thread**: Cross-platform threading (threads, mutexes, condition variables)
 * - **Logger**: Thread-safe logging with multiple severity levels
 * - **Profiler**: Stack-based function call profiler for debugging
 * - **Event**: Event system for decoupled communication
 * - **Graphics**: Dedicated rendering thread with OpenGL support
 * - **Application**: Main game loop with fixed timestep
 * - **Container**: Thread-safe data structures (queue, stack, etc.)
 *
 * @section initialization Initialization Order
 * The engine must be initialized in this order:
 * 1. tl_platform_initialize() - Initialize platform layer
 * 2. tl_application_initialize() - Initialize application
 * 3. tl_application_run() - Run main game loop
 * 4. tl_application_terminate() - Cleanup application
 * 5. tl_platform_terminate() - Cleanup platform layer
 *
 * @section example Usage Example
 * @code
 * #include "teleios/teleios.h"
 *
 * int main(void) {
 *     tl_logger_loglevel(TL_LOG_LEVEL_DEBUG);
 *
 *     if (!tl_platform_initialize()) {
 *         return 1;
 *     }
 *
 *     if (!tl_application_initialize()) {
 *         tl_platform_terminate();
 *         return 1;
 *     }
 *
 *     if (!tl_application_run()) {
 *         tl_application_terminate();
 *         tl_platform_terminate();
 *         return 1;
 *     }
 *
 *     tl_application_terminate();
 *     tl_platform_terminate();
 *     return 0;
 * }
 * @endcode
 *
 * @author TELEIOS Team
 * @version 0.1.0
 * @date 2025
 */

#ifndef __TELEIOS__
#define __TELEIOS__

#include "teleios/defines.h"
#include "teleios/profiler.h"
#include "teleios/platform.h"
#include "teleios/memory.h"
#include "teleios/window.h"
#include "teleios/thread.h"

#include "teleios/container.h"

#include "teleios/number.h"
#include "teleios/event.h"
#include "teleios/chrono.h"
#include "teleios/logger.h"
#include "teleios/filesystem.h"
#include "teleios/graphics.h"

#endif
