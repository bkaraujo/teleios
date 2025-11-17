#include "teleios/teleios.h"

u32 tl_number_next_power_of_2(u32 n) {
    TL_PROFILER_PUSH_WITH("%u", n)

    if (n == 0) TL_PROFILER_POP_WITH(1)

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    TL_PROFILER_POP_WITH(n)
}

#include "teleios/number/signed.inl"
#include "teleios/number/unsigned.inl"
