#include "teleios/core/platform_detector.h"
#ifdef TLPLATFORM_WINDOWS
#include "teleios/core.h"

// ########################################################
//                    TIME FUNCTIONS
// ########################################################
u64 tl_time_epoch(void) {
    FILETIME ft;
    ULARGE_INTEGER uli;
    uint64_t epoch_microseconds;

    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // FILETIME is in 100-nanosecond intervals since January 1, 1601 (UTC).
    // We need to convert it to microseconds since January 1, 1970 (UTC).

    // Subtract the number of 100-nanosecond intervals between the two dates.
    // The value is 116444736000000000 (obtained from various sources).
    epoch_microseconds = (uli.QuadPart - 116444736000000000ULL) / 10; // Convert to microseconds

    return epoch_microseconds;
}

#endif