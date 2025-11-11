/**
 * @file chrono.h
 * @brief High-resolution timing and date/time utilities
 *
 * This module provides access to the system clock for timing operations and
 * retrieving current date/time information. It provides three core timing functions:
 *
 * 1. **Date/Time Retrieval**: Get current year, month, day, hour, minute, second
 * 2. **Millisecond Precision**: Elapsed time since Unix epoch in milliseconds
 * 3. **Microsecond Precision**: Elapsed time since Unix epoch in microseconds
 *
 * The timing functions are platform-specific:
 * - **Windows**: Uses QueryPerformanceCounter for high resolution
 * - **Linux**: Uses clock_gettime() with CLOCK_REALTIME
 * - **macOS**: Uses platform-specific timing
 *
 * @section timestamp_constant Timestamp Conversion
 * The constant ONE_SECOND_MICROS (1000000.0) is provided for converting
 * between microseconds and seconds.
 *
 * @section usage Usage Examples
 *
 * **Getting current date and time:**
 * @code
 * #include "teleios/chrono.h"
 *
 * TLDateTime now;
 * tl_time_clock(&now);
 *
 * printf("Current time: %04u-%02u-%02u %02u:%02u:%02u.%03u\n",
 *     now.year, now.month, now.day,
 *     now.hour, now.minute, now.second, now.millis);
 *
 * // Example output: Current time: 2025-11-15 14:30:45.123
 * @endcode
 *
 * **Measuring elapsed time in milliseconds:**
 * @code
 * u64 start_ms = tl_time_epoch_millis();
 *
 * // ... do some work ...
 *
 * u64 end_ms = tl_time_epoch_millis();
 * u64 elapsed_ms = end_ms - start_ms;
 *
 * printf("Operation took %llu milliseconds\n", elapsed_ms);
 * @endcode
 *
 * **Measuring elapsed time in microseconds:**
 * @code
 * u64 start_us = tl_time_epoch_micros();
 *
 * // ... perform timing-sensitive operation ...
 *
 * u64 end_us = tl_time_epoch_micros();
 * f64 elapsed_seconds = (f64)(end_us - start_us) / ONE_SECOND_MICROS;
 *
 * printf("Operation took %.6f seconds\n", elapsed_seconds);
 * @endcode
 *
 * **Frame timing for game loops:**
 * @code
 * u64 frame_start = tl_time_epoch_micros();
 * u64 prev_time = frame_start;
 *
 * while (running) {
 *     u64 current_time = tl_time_epoch_micros();
 *     f64 delta_time = (f64)(current_time - prev_time) / ONE_SECOND_MICROS;
 *     prev_time = current_time;
 *
 *     update_game(delta_time);
 *     render_game();
 * }
 * @endcode
 *
 * @note The logger uses tl_time_clock() internally to timestamp each log message.
 *
 * @note For performance-critical code, cache the result of tl_time_epoch_micros()
 *       to avoid repeated syscalls. The logger uses thread-local timestamp caching
 *       to minimize overhead.
 *
 * @note Date/time values may not be perfectly synchronized if called from different
 *       threads. Use epoch times for cross-thread timing measurements.
 *
 * @see logger.h - Uses tl_time_clock() for logging timestamps
 * @see platform.h - Platform initialization that sets up timing
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

#ifndef __TELEIOS_CHRONO__
#define __TELEIOS_CHRONO__

#include "teleios/defines.h"

/**
 * @brief Current date and time representation
 *
 * Holds a breakdown of the current date and time into individual components.
 * This structure is returned by tl_time_clock().
 *
 * @note Year is stored as a full 4-digit year (e.g., 2025)
 * @note Month is 1-12 (not 0-11)
 * @note Day is 1-31
 * @note Hour is 0-23
 * @note Minute is 0-59
 * @note Second is 0-59
 * @note Millis is 0-999
 *
 * @see tl_time_clock
 */
typedef struct {
    u16 millis;                 ///< Milliseconds (0-999)
    u16 year;                   ///< Full year (e.g., 2025)
    u8 month;                   ///< Month (1-12)
    u8 day;                     ///< Day of month (1-31)
    u8 hour;                    ///< Hour (0-23)
    u8 minute;                  ///< Minute (0-59)
    u8 second;                  ///< Second (0-59)
} TLDateTime;

/**
 * @brief Constant for converting microseconds to seconds
 *
 * Use this constant to convert microsecond measurements to fractional seconds.
 *
 * @example
 * u64 micros = tl_time_epoch_micros();
 * f64 seconds = micros / ONE_SECOND_MICROS;
 */
static const f64 ONE_SECOND_MICROS = 1000000.0;     ///< 1 second in microseconds

/**
 * @brief Get current date and time
 *
 * Retrieves the current date and time from the system clock and fills the
 * provided TLDateTime structure with individual date/time components.
 *
 * @param clock Pointer to TLDateTime structure to fill (must not be NULL)
 *
 * @note Thread-safe on all platforms
 * @note The timestamp represents the moment this function is called
 * @note Year is full 4-digit value (2025, not 125)
 * @note Month is 1-12 (not 0-11)
 *
 * @see TLDateTime
 * @see tl_time_epoch_millis
 * @see tl_time_epoch_micros
 *
 * @code
 * TLDateTime now;
 * tl_time_clock(&now);
 *
 * // Log current time
 * TLINFO("Current date: %04u-%02u-%02u", now.year, now.month, now.day);
 * TLINFO("Current time: %02u:%02u:%02u", now.hour, now.minute, now.second);
 * @endcode
 */
void tl_time_clock(TLDateTime* clock);

/**
 * @brief Get milliseconds since Unix epoch (January 1, 1970 00:00:00 UTC)
 *
 * Returns the number of milliseconds elapsed since the Unix epoch. This is useful
 * for measuring elapsed time with millisecond precision.
 *
 * @return Milliseconds since Unix epoch (1970-01-01 00:00:00 UTC)
 *
 * @note Thread-safe on all platforms
 * @note Millisecond precision (but actual resolution may vary by platform)
 * @note On 64-bit systems, this won't overflow for ~292 million years
 * @note For sub-millisecond timing, use tl_time_epoch_micros() instead
 *
 * @see tl_time_epoch_micros - For microsecond precision
 * @see tl_time_clock - For human-readable date/time
 *
 * @code
 * // Time an operation
 * u64 start = tl_time_epoch_millis();
 *
 * perform_task();
 *
 * u64 elapsed = tl_time_epoch_millis() - start;
 * TLINFO("Task completed in %llu milliseconds", elapsed);
 * @endcode
 */
u64 tl_time_epoch_millis(void);

/**
 * @brief Get microseconds since Unix epoch (January 1, 1970 00:00:00 UTC)
 *
 * Returns the number of microseconds elapsed since the Unix epoch. This is useful
 * for high-precision timing measurements.
 *
 * @return Microseconds since Unix epoch (1970-01-01 00:00:00 UTC)
 *
 * @note Thread-safe on all platforms
 * @note Microsecond precision (but actual resolution may vary by platform)
 * @note On 64-bit systems, this won't overflow for ~292,000 years
 * @note For lower precision with less overhead, use tl_time_epoch_millis()
 *
 * @see tl_time_epoch_millis - For millisecond precision
 * @see tl_time_clock - For human-readable date/time
 * @see ONE_SECOND_MICROS - Constant for microsecond-to-second conversion
 *
 * @code
 * // High-precision frame timing
 * u64 frame_start = tl_time_epoch_micros();
 * u64 prev_frame = frame_start;
 *
 * while (running) {
 *     u64 now = tl_time_epoch_micros();
 *     f64 delta_seconds = (f64)(now - prev_frame) / ONE_SECOND_MICROS;
 *     prev_frame = now;
 *
 *     update(delta_seconds);
 *
 *     f64 frame_time = (f64)(now - frame_start) / ONE_SECOND_MICROS;
 *     if (frame_time >= 1.0) {
 *         TLINFO("Frame rate: %.1f FPS", frame_count / frame_time);
 *         frame_count = 0;
 *         frame_start = now;
 *     }
 * }
 * @endcode
 */
u64 tl_time_epoch_micros(void);

#endif
