#ifndef __TELEIOS_CORE_NUMBER__
#define __TELEIOS_CORE_NUMBER__

#include "teleios/defines.h"

/**
 * Identify the number of digits in a particular i32 number.
 * For instance, the number 25 has 2 digits.
 *
 * @param number The desired number
 * @return The number of digits
 */
u16 tl_number_i32_digits(i32 number);

#endif //__TELEIOS_CORE_NUMBER__
