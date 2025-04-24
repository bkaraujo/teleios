#ifndef __LIBK_NUMBER__
#define __LIBK_NUMBER__

#include "libk/defines.h"

/**
 * Identify the number of digits in a particular i32 number.
 * For instance, the number 25 has 2 digits.
 *
 * @param number The desired number
 * @return The number of digits
 */
u16 k_number_i32_digits(i32 number);

#endif
