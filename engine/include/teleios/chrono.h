#ifndef __TELEIOS_CHRONO__
#define __TELEIOS_CHRONO__

#include "teleios/defines.h"

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
