#ifndef __TELEIOS_LOGGER__
#define __TELEIOS_LOGGER__

#include "teleios/defines.h"

/**
 * @brief Set the global logging level threshold
 *
 * Only log messages with severity >= this level will be output. This allows
 * you to control verbosity at runtime.
 *
 * @param desired The desired minimum log level
 *
 * @note Default level is typically TL_LOG_LEVEL_INFO
 *
 * @see TLLogLevel
 * @see tl_logger_write
 *
 * @code
 * // Enable very verbose logging for debugging
 * tl_logger_loglevel(TL_LOG_LEVEL_TRACE);
 *
 * // Reduce verbosity in production
 * tl_logger_loglevel(TL_LOG_LEVEL_WARN);
 * @endcode
 */
void tl_logger_set_level(TLLogLevel desired);
TLLogLevel tl_logger_get_level(void);

/**
 * @brief Write a log message to the logger
 *
 * This is the core logging function. It is typically called through convenience
 * macros (TLINFO, TLERROR, etc.) rather than directly.
 *
 * The function formats the message, extracts the filename from the full path,
 * retrieves the current timestamp, and writes the formatted output to stdout.
 *
 * @param level The log level (severity) of this message
 * @param filename The source file that generated the log (use __FILE__)
 * @param lineno The line number in the source file (use __LINE__)
 * @param message The format string (printf-style)
 * @param ... Format string arguments
 *
 * @note This function is thread-safe. Each thread maintains its own timestamp cache.
 *
 * @note The message is only output if level >= the configured threshold level.
 *
 * @see tl_logger_loglevel
 * @see TLLogLevel
 *
 * @code
 * // Direct call (not recommended - use macros instead)
 * tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Value: %d", 42);
 *
 * // Recommended: Use convenience macros
 * TLINFO("Value: %d", 42);  // Automatically includes __FILE__ and __LINE__
 * @endcode
 */
void tl_logger_write(TLLogLevel level, const char *filename, u32 lineno, const char *message, ...);

/**
 * @brief Log very detailed diagnostic information
 *
 * This macro logs at VERBOSE level and is compiled out in Release builds.
 * Use for low-level detailed diagnostics that are only needed during development.
 *
 * @param m The format string (printf-style)
 * @param ... Format arguments
 *
 * @note Compiled out in Release builds (TELEIOS_BUILD_RELEASE)
 * @note Zero overhead when compiled out
 *
 * @see TL_LOG_LEVEL_VERBOSE
 * @see TLTRACE
 * @see TLDEBUG
 *
 * @code
 * TLVERBOSE("Array index: %d, value: %f", i, array[i]);
 * TLVERBOSE("Pointer comparison:0x%p ==0x%p: %s", ptr1, ptr2, ptr1 == ptr2 ? "yes" : "no");
 * @endcode
 */
#if ! defined(TELEIOS_BUILD_RELEASE)
#   define TLVERBOSE(m, ...) { tl_logger_write(TL_LOG_LEVEL_VERBOSE, __FILE__, __LINE__, m, ##__VA_ARGS__); }
#else
#   define TLVERBOSE(m, ...)
#endif

/**
 * @brief Log function entry/exit and control flow
 *
 * This macro logs at TRACE level and is compiled out in Release builds.
 * Use for tracing execution flow and understanding call stacks during debugging.
 *
 * @param m The format string (printf-style)
 * @param ... Format arguments
 *
 * @note Compiled out in Release builds (TELEIOS_BUILD_RELEASE)
 * @note Zero overhead when compiled out
 *
 * @see TL_LOG_LEVEL_TRACE
 * @see TLVERBOSE
 * @see TLDEBUG
 *
 * @code
 * void process_entity(Entity* e) {
 *     TLTRACE("Processing entity %u", e->id);
 *     // ... processing ...
 *     TLTRACE("Entity %u processing complete", e->id);
 * }
 * @endcode
 */
#if ! defined(TELEIOS_BUILD_RELEASE)
#   define   TLTRACE(m, ...) { tl_logger_write(TL_LOG_LEVEL_TRACE  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#else
#   define   TLTRACE(m, ...)
#endif

/**
 * @brief Log general debugging messages
 *
 * This macro logs at DEBUG level and is compiled out in Release builds.
 * Use for debugging information that helps understand application behavior.
 *
 * @param m The format string (printf-style)
 * @param ... Format arguments
 *
 * @note Compiled out in Release builds (TELEIOS_BUILD_RELEASE)
 * @note Zero overhead when compiled out
 *
 * @see TL_LOG_LEVEL_DEBUG
 * @see TLVERBOSE
 * @see TLTRACE
 *
 * @code
 * TLDEBUG("Loaded %d entities from scene", entity_count);
 * TLDEBUG("Physics step: dt=%.4f, frame=%u", delta_time, frame_number);
 * @endcode
 */
#if ! defined(TELEIOS_BUILD_RELEASE)
#   define   TLDEBUG(m, ...) { tl_logger_write(TL_LOG_LEVEL_DEBUG  , __FILE__, __LINE__, m, ##__VA_ARGS__); }
#else
#   define   TLDEBUG(m, ...)
#endif

/**
 * @brief Log informational messages
 *
 * This macro logs at INFO level. Use for messages about normal application
 * operation that are important but not errors or warnings.
 *
 * @param m The format string (printf-style)
 * @param ... Format arguments
 *
 * @note Always enabled, even in Release builds
 *
 * @see TL_LOG_LEVEL_INFO
 * @see TLWARN
 * @see TLERROR
 *
 * @code
 * TLINFO("Application initialized");
 * TLINFO("Loaded scene: %s", scene_name);
 * TLINFO("FPS: %.1f, Frame time: %.2f ms", fps, frame_time_ms);
 * @endcode
 */
#define    TLINFO(m, ...) { tl_logger_write(TL_LOG_LEVEL_INFO   , __FILE__, __LINE__, m, ##__VA_ARGS__); }

/**
 * @brief Log warning messages
 *
 * This macro logs at WARN level. Use for conditions that are unexpected but
 * don't necessarily prevent the application from continuing.
 *
 * @param m The format string (printf-style)
 * @param ... Format arguments
 *
 * @note Always enabled, even in Release builds
 *
 * @see TL_LOG_LEVEL_WARN
 * @see TLINFO
 * @see TLERROR
 *
 * @code
 * if (entity_count > 1000) {
 *     TLWARN("High entity count: %d (may impact performance)", entity_count);
 * }
 *
 * if (memory_used > memory_limit * 0.9f) {
 *     TLWARN("Memory usage at %.1f%% of limit", (memory_used / memory_limit) * 100.0f);
 * }
 * @endcode
 */
#define    TLWARN(m, ...) { tl_logger_write(TL_LOG_LEVEL_WARN   , __FILE__, __LINE__, m, ##__VA_ARGS__); }

/**
 * @brief Log error messages
 *
 * This macro logs at ERROR level. Use for error conditions that indicate
 * something went wrong but the application can attempt recovery.
 *
 * @param m The format string (printf-style)
 * @param ... Format arguments
 *
 * @note Always enabled, even in Release builds
 * @note Does not terminate the program
 *
 * @see TL_LOG_LEVEL_ERROR
 * @see TLWARN
 * @see TLFATAL
 *
 * @code
 * if (!load_texture(filename)) {
 *     TLERROR("Failed to load texture: %s", filename);
 *     return false;  // Caller handles error
 * }
 *
 * if (file == NULL) {
 *     TLERROR("Cannot open file: %s (errno: %d)", path, errno);
 * }
 * @endcode
 */
#define   TLERROR(m, ...) { tl_logger_write(TL_LOG_LEVEL_ERROR  , __FILE__, __LINE__, m, ##__VA_ARGS__); }

/**
 * @brief Log fatal errors and terminate program
 *
 * This macro logs at FATAL level and then exits the program with code 99.
 * Use only for unrecoverable errors that prevent continuation.
 *
 * @param m The format string (printf-style)
 * @param ... Format arguments
 *
 * @note NEVER RETURNS - Calls exit(99) immediately after logging
 * @note Always enabled, even in Release builds
 *
 * @see TL_LOG_LEVEL_FATAL
 * @see TLERROR
 *
 * @code
 * // OpenGL initialization failed - cannot continue
 * if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
 *     TLFATAL("Failed to initialize OpenGL: %s", gl_error_message);
 *     // Never reached - exit(99) was called
 * }
 *
 * // Critical resource unavailable
 * if (critical_allocator == NULL) {
 *     TLFATAL("Memory system initialization failed");
 * }
 * @endcode
 */
#define   TLFATAL(m, ...) { tl_logger_write(TL_LOG_LEVEL_FATAL  , __FILE__, __LINE__, m, ##__VA_ARGS__); exit(99); }

#endif
