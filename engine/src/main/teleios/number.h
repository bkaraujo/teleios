#ifndef __TELEIOS_NUMBER__
#define __TELEIOS_NUMBER__

#include "teleios/defines.h"

/**
 * @brief Count the number of digits in a signed 32-bit integer
 *
 * Determines how many digit characters are needed to represent the number
 * in decimal form. For negative numbers, includes the minus sign in the count.
 *
 * @param number The number to count digits for
 * @return Number of digits (including minus sign for negative numbers)
 *
 * @note For number 0, returns 1
 * @note For negative numbers, count includes the minus sign
 * @note Example: -42 returns 3 (the '-', '4', and '2')
 * @note Useful for calculating buffer sizes before string conversion
 *
 * @see tl_number_i32_to_char - Convert number to string
 *
 * @code
 * u8 digits;
 *
 * digits = tl_number_i32_digits(0);          // Returns 1
 * digits = tl_number_i32_digits(5);          // Returns 1
 * digits = tl_number_i32_digits(42);         // Returns 2
 * digits = tl_number_i32_digits(12345);      // Returns 5
 * digits = tl_number_i32_digits(-42);        // Returns 3 (includes minus)
 * digits = tl_number_i32_digits(2147483647); // Returns 10
 * digits = tl_number_i32_digits(-2147483648);// Returns 11 (max negative)
 *
 * // Use for buffer allocation
 * i32 value = 12345;
 * u8 num_digits = tl_number_i32_digits(value);
 * char buffer[num_digits + 1];  // +1 for null terminator
 * tl_number_i32_to_char(buffer, value, 10);
 * @endcode
 */
u8 tl_number_i32_digits(i32 number);

/**
 * @brief Convert a signed 32-bit integer to a null-terminated string
 *
 * Converts an i32 value to its string representation in the specified base
 * (radix). The output is null-terminated and may include a minus sign for
 * negative numbers.
 *
 * Supports bases from 2 (binary) through 36, with digits 0-9 and letters
 * a-z for bases > 10. The output uses lowercase letters.
 *
 * @param buffer Pointer to output buffer (must be large enough)
 * @param value The number to convert
 * @param base The radix/base for conversion (2-36)
 *
 * @note Caller must ensure buffer is large enough for result + null terminator
 * @note For maximum i32 (including minus sign), need minimum 12 bytes
 * @note A 256-byte buffer is always safe for any i32 value
 * @note Output is null-terminated (caller must provide space for '\0')
 * @note Bases > 10 use lowercase letters: 10='a', 15='f', etc.
 * @note Negative numbers include minus sign in output
 *
 * @see tl_number_i32_digits - Determine required buffer size
 *
 * @code
 * char buffer[256];
 *
 * // Common conversions
 * tl_number_i32_to_char(buffer, 255, 2);    // "11111111"
 * tl_number_i32_to_char(buffer, 255, 8);    // "377"
 * tl_number_i32_to_char(buffer, 255, 10);   // "255"
 * tl_number_i32_to_char(buffer, 255, 16);   // "ff"
 *
 * // Negative numbers
 * tl_number_i32_to_char(buffer, -42, 10);   // "-42"
 *
 * // With digit counting
 * i32 num = 1234567;
 * u8 digits = tl_number_i32_digits(num);
 * char small_buffer[digits + 1];  // Exact size
 * tl_number_i32_to_char(small_buffer, num, 10);
 * TLINFO("Number: %s", small_buffer);  // "1234567"
 *
 * // Base 36 (maximum with letters)
 * tl_number_i32_to_char(buffer, 35, 36);   // "z"
 * tl_number_i32_to_char(buffer, 1295, 36); // "zzz" (35*36^2 + 35*36 + 35)
 * @endcode
 */
TLString* tl_number_i32_to_char(TLAllocator* allocator, i32 value, u8 base);

#endif
