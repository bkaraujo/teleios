/**
 * @file profiler.h
 * @brief Stack-based function call profiler for call hierarchy tracing and debugging
 *
 * This module provides a lightweight call stack profiler for understanding program
 * execution flow and identifying performance bottlenecks. The profiler is stack-based,
 * meaning it tracks nested function calls in a call hierarchy.
 *
 * Key features:
 * - Stack-based call tracking (up to 1000 nested levels)
 * - Automatic filename, function name, and line number capture
 * - Optional argument logging for parameter values
 * - Entry/exit logging through the logger subsystem
 * - Zero overhead in Release builds (completely compiled out)
 * - Thread-local storage for thread-safe operation
 *
 * @section call_stack Call Stack Architecture
 *
 * The profiler maintains a stack of active function frames:
 *
 * ```
 *  Frame 4: function_d(...)  [deepest]
 *  Frame 3: function_c(...)
 *  Frame 2: function_b(...)
 *  Frame 1: function_a(...)  [shallowest]
 *  Frame 0: main()
 * ```
 *
 * When you push a frame, it's added to the top of the stack. When you pop,
 * the current frame is removed and execution returns to the previous frame.
 *
 * @section performance Performance Characteristics
 *
 * Performance metrics when profiling is enabled (TELEIOS_BUILD_DEBUG):
 * - Memory usage: ~1 KB per frame (1024 bytes args buffer + metadata)
 * - Stack depth: 1000 frames max (configurable via TELEIOS_FRAME_MAXIMUM)
 * - Total memory: ~1.03 MB for 1000 max frames
 * - Overhead per push: < 1 microsecond (stack allocation only)
 * - Overhead per pop: < 1 microsecond (with optional logging)
 *
 * Key optimizations:
 * - Static allocation (no malloc/free overhead in hot path)
 * - Branch prediction hints (TL_LIKELY/TL_UNLIKELY) for common paths
 * - Direct pointer assignment (no string copying)
 * - Arguments buffer caching: 1024 bytes per frame (configurable)
 *
 * @section limitations Limitations and Constraints
 *
 * **Call Stack Depth:**
 * - Default maximum: 1000 frames (configurable via TELEIOS_FRAME_MAXIMUM)
 * - Exceeding limit causes TLFATAL() - application terminates
 * - No graceful degradation: cannot continue with deeper stacks
 * - Suitable for normal call chains but not pathological recursion
 *
 * **Arguments Buffer:**
 * - 1024 bytes per frame (configurable via TL_PROFILER_FRAME_ARGUMENTS_SIZE)
 * - Stores formatted argument strings
 * - Overflow causes truncation (no error)
 *
 * **Production Use:**
 * - Completely compiled out in Release builds (zero overhead)
 * - Debug builds have measurable overhead from logging
 * - Consider external tools (Tracy, Optick) for production profiling
 *
 * @section macros Profiler Macros
 *
 * Four main profiler macros simplify frame management:
 *
 * **TL_PROFILER_PUSH** - Enter a function without arguments
 * - Logs entry point
 * - Pushes frame to call stack
 * - Use at the beginning of functions
 *
 * **TL_PROFILER_PUSH_WITH(format, ...)** - Enter with argument values
 * - Logs entry point with formatted arguments
 * - Useful for understanding parameter values
 * - Supports printf-style format strings
 *
 * **TL_PROFILER_POP** - Exit a void function
 * - Pops frame from call stack
 * - Logs exit point
 * - Includes return statement (void return)
 *
 * **TL_PROFILER_POP_WITH(value)** - Exit with return value
 * - Pops frame and returns specified value
 * - Logs exit point with return value
 * - Includes return statement
 *
 * @section usage Usage Examples
 *
 * **Basic function tracing:**
 * @code
 * void process_entities(void) {
 *     TL_PROFILER_PUSH
 *
 *     for (u32 i = 0; i < entity_count; ++i) {
 *         update_entity(&entities[i]);
 *     }
 *
 *     TL_PROFILER_POP  // Void return
 * }
 * @endcode
 *
 * **Tracing with parameter logging:**
 * @code
 * float calculate_velocity(float acceleration, float time) {
 *     TL_PROFILER_PUSH_WITH("accel=%.2f, time=%.3f", acceleration, time)
 *
 *     float velocity = acceleration * time;
 *
 *     TL_PROFILER_POP_WITH(velocity)  // Returns float value
 * }
 * @endcode
 *
 * **Tracing recursive functions:**
 * @code
 * int fibonacci(int n) {
 *     TL_PROFILER_PUSH_WITH("n=%d", n)
 *
 *     if (n <= 1) {
 *         TL_PROFILER_POP_WITH(n)
 *     }
 *
 *     int result = fibonacci(n - 1) + fibonacci(n - 2);
 *     TL_PROFILER_POP_WITH(result)
 * }
 * @endcode
 *
 * **Conditional profiling:**
 * @code
 * #if defined(TELEIOS_BUILD_DEBUG)
 * void debug_helper(void) {
 *     TL_PROFILER_PUSH
 *     // ... debugging code ...
 *     TL_PROFILER_POP
 * }
 * #endif
 * @endcode
 *
 * **Profiling with logger integration:**
 * @code
 * void complex_operation(const char* resource_name) {
 *     TL_PROFILER_PUSH_WITH("resource='%s'", resource_name)
 *
 *     TLDEBUG("Starting complex operation on resource: %s", resource_name);
 *
 *     // ... perform work ...
 *
 *     TLDEBUG("Complex operation completed");
 *     TL_PROFILER_POP
 * }
 * @endcode
 *
 * @section output Log Output
 *
 * The profiler generates [in] and [out] tags in the logger output:
 *
 * ```
 * [in]  main:23 main()
 * [in]  application.c:156 tl_application_run()
 * [in]  game.c:89 update_game(delta=0.016667)
 * [out] game.c:120 update_game()
 * [out] application.c:200 tl_application_run()
 * [out] main:30 main()
 * ```
 *
 * The output shows:
 * - Entry ([in]) and exit ([out]) markers
 * - Source file and line number
 * - Function name and arguments
 *
 * @note In Release builds, all profiler macros expand to nothing.
 *       Production builds have zero profiling overhead.
 *
 * @note Arguments are formatted into a 1024-byte buffer. Very long argument
 *       strings may be truncated. For extremely complex arguments, log
 *       them separately using TLDEBUG() instead.
 *
 * @note The profiler is thread-local by default. Each thread has its own
 *       call stack, so profiling is thread-safe.
 *
 * @see logger.h - Logger subsystem (TLDEBUG, etc.)
 * @see defines.h - Compiler macros (TL_LIKELY, TL_UNLIKELY)
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

#ifndef __TELEIOS_PROFILER__
#define __TELEIOS_PROFILER__

#include "teleios/defines.h"
#include "teleios/logger.h"

/**
 * @brief Pop current frame from call stack and log exit
 *
 * This is the core profiler function for exiting a function frame.
 * Typically called through TL_PROFILER_POP or TL_PROFILER_POP_WITH macros.
 *
 * The function:
 * - Removes the current frame from the call stack
 * - Logs the exit point with TLDEBUG
 * - Updates the active frame pointer
 *
 * @note Directly calling this is not recommended. Use the TL_PROFILER_POP
 *       or TL_PROFILER_POP_WITH macros instead.
 *
 * @see tl_profiler_frame_push
 * @see TL_PROFILER_POP
 * @see TL_PROFILER_POP_WITH
 */
void tl_profiler_frame_pop();

/**
 * @brief Push new function frame to call stack and log entry
 *
 * This is the core profiler function for entering a function frame.
 * Typically called through TL_PROFILER_PUSH or TL_PROFILER_PUSH_WITH macros.
 *
 * The function:
 * - Checks if call stack depth hasn't exceeded the maximum
 * - Creates a new frame at the top of the stack
 * - Logs the entry point with TLDEBUG
 * - Optionally formats argument values into the frame
 *
 * @param filename Source file path (typically __FILE__)
 * @param lineno Line number (typically __LINE__)
 * @param function Function name (typically __func__)
 * @param arguments Format string for argument values (may be NULL)
 * @param ... Arguments to format (if arguments format string provided)
 *
 * @note Directly calling this is not recommended. Use the TL_PROFILER_PUSH
 *       or TL_PROFILER_PUSH_WITH macros instead.
 *
 * @warning If call stack depth exceeds TELEIOS_FRAME_MAXIMUM (default 1000),
 *          the application will terminate with TLFATAL. There is no recovery.
 *
 * @see tl_profiler_frame_pop
 * @see TL_PROFILER_PUSH
 * @see TL_PROFILER_PUSH_WITH
 */
void tl_profiler_frame_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);

/**
 * @brief Enter a function for profiling (no arguments)
 *
 * Pushes a frame to the call stack and logs entry. Use at the beginning
 * of a function body for unconditional entry logging.
 *
 * Must be paired with TL_PROFILER_POP or TL_PROFILER_POP_WITH before
 * returning from the function.
 *
 * @note Compiled out in Release builds (TELEIOS_BUILD_RELEASE)
 * @note Zero overhead when compiled out
 * @note Expands to nothing if TELEIOS_BUILD_DEBUG is not defined
 *
 * @see tl_profiler_frame_push
 * @see TL_PROFILER_PUSH_WITH
 * @see TL_PROFILER_POP
 *
 * @code
 * void my_function(void) {
 *     TL_PROFILER_PUSH
 *
 *     // ... function body ...
 *
 *     TL_PROFILER_POP  // Must use POP macro for proper return
 * }
 * @endcode
 */
#if defined(TELEIOS_BUILD_DEBUG)
#   define TL_PROFILER_PUSH tl_profiler_frame_push(__FILE__, __LINE__, __func__, NULL);
#else
#   define TL_PROFILER_PUSH
#endif

/**
 * @brief Enter a function with argument logging
 *
 * Pushes a frame to the call stack and logs entry with formatted arguments.
 * Use at the beginning of a function to capture parameter values for debugging.
 *
 * The format string and arguments are formatted using printf-style formatting.
 * The formatted result is stored in the frame for logging.
 *
 * @param args Format string for arguments (printf-style)
 * @param ... Arguments to format
 *
 * @note Compiled out in Release builds (TELEIOS_BUILD_RELEASE)
 * @note Zero overhead when compiled out
 * @note Arguments buffer is 1024 bytes; very long arguments may be truncated
 *
 * @see tl_profiler_frame_push
 * @see TL_PROFILER_PUSH
 * @see TL_PROFILER_POP_WITH
 *
 * @code
 * float distance(float x1, float y1, float x2, float y2) {
 *     TL_PROFILER_PUSH_WITH("x1=%.1f, y1=%.1f, x2=%.1f, y2=%.1f", x1, y1, x2, y2)
 *
 *     float dx = x2 - x1;
 *     float dy = y2 - y1;
 *     float result = sqrtf(dx*dx + dy*dy);
 *
 *     TL_PROFILER_POP_WITH(result)
 * }
 * @endcode
 */
#if defined(TELEIOS_BUILD_DEBUG)
#   define TL_PROFILER_PUSH_WITH(args, ...) tl_profiler_frame_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);
#else
#   define TL_PROFILER_PUSH_WITH(args, ...)
#endif

/**
 * @brief Exit a void function with profiling
 *
 * Pops the current frame from the call stack, logs exit, and returns.
 * Must be paired with a previous TL_PROFILER_PUSH or TL_PROFILER_PUSH_WITH.
 *
 * This macro includes the return statement, so you should use it instead of
 * writing a separate return statement at the end of void functions.
 *
 * @note Compiled out in Release builds (TELEIOS_BUILD_RELEASE)
 * @note Zero overhead when compiled out
 * @note Includes implicit return - don't add another return after this macro
 * @note For functions returning a value, use TL_PROFILER_POP_WITH instead
 *
 * @see tl_profiler_frame_pop
 * @see TL_PROFILER_PUSH
 * @see TL_PROFILER_PUSH_WITH
 * @see TL_PROFILER_POP_WITH
 *
 * @code
 * void process_data(void) {
 *     TL_PROFILER_PUSH
 *
 *     // ... processing ...
 *
 *     TL_PROFILER_POP  // Pops frame and returns
 * }
 * @endcode
 */
#if defined(TELEIOS_BUILD_DEBUG)
#   define TL_PROFILER_POP do { tl_profiler_frame_pop(); return; } while (false);
#else
#   define TL_PROFILER_POP return;
#endif

/**
 * @brief Exit a function with return value and profiling
 *
 * Pops the current frame from the call stack, logs exit with the return value,
 * and returns the specified value. Must be paired with a previous TL_PROFILER_PUSH
 * or TL_PROFILER_PUSH_WITH.
 *
 * This macro includes the return statement and value, so you should use it instead
 * of writing a separate return statement. The return value is passed as an argument
 * to this macro.
 *
 * @param v The value to return
 *
 * @note Compiled out in Release builds (TELEIOS_BUILD_RELEASE)
 * @note Zero overhead when compiled out
 * @note Includes implicit return - don't add another return after this macro
 * @note For void functions, use TL_PROFILER_POP instead
 *
 * @see tl_profiler_frame_pop
 * @see TL_PROFILER_PUSH_WITH
 * @see TL_PROFILER_POP
 *
 * @code
 * int calculate_sum(int a, int b) {
 *     TL_PROFILER_PUSH_WITH("a=%d, b=%d", a, b)
 *
 *     int result = a + b;
 *
 *     TL_PROFILER_POP_WITH(result)  // Returns result value
 * }
 *
 * bool validate_range(float value, float min, float max) {
 *     TL_PROFILER_PUSH_WITH("value=%.2f, range=[%.2f, %.2f]", value, min, max)
 *
 *     bool is_valid = (value >= min && value <= max);
 *
 *     TL_PROFILER_POP_WITH(is_valid)
 * }
 * @endcode
 */
#if defined(TELEIOS_BUILD_DEBUG)
#   define TL_PROFILER_POP_WITH(v) do { tl_profiler_frame_pop(); return v; } while (false);
#else
#   define TL_PROFILER_POP_WITH(v) return v;
#endif

#endif
