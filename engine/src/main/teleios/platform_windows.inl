#ifndef __TELEIOS_PLATFORM_WINDOWS__
#define __TELEIOS_PLATFORM_WINDOWS__

#include "teleios/defines.h"

#ifdef TL_PLATFORM_WINDOWS

#include "teleios/teleios.h"
#include <windows.h>
#include <direct.h>

// ---------------------------------
// Windows Platform - State
// ---------------------------------

static u64 qpc_freq = 0;            // Clock frequency
static u64 qpc_to_micros_mul = 0;   // Pre-calculated multiplier
static u64 qpc_to_micros_shift = 0; // Bit shift for precision
static u64 qpc_epoch_offset = 0;    // Offset to Unix epoch

// ---------------------------------
// Windows Platform - Initialization
// ---------------------------------

static b8 tl_winapi_initialize(void) {
    TL_PROFILER_PUSH

    // Get QueryPerformanceCounter frequency
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    qpc_freq = freq.QuadPart;

    // Pre-calculate multiplier to avoid division in hot path
    // Using 20-bit shift for precision
    qpc_to_micros_shift = 20;
    qpc_to_micros_mul = (1000000ULL << qpc_to_micros_shift) / qpc_freq;

    // Calculate offset to Unix epoch
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    u64 epoch_micros = (uli.QuadPart - 116444736000000000ULL) / 10;

    LARGE_INTEGER qpc_now;
    QueryPerformanceCounter(&qpc_now);
    qpc_epoch_offset = epoch_micros - ((qpc_now.QuadPart * qpc_to_micros_mul) >> qpc_to_micros_shift);

    TL_PROFILER_POP_WITH(true)
}

static b8 tl_winapi_terminate(void) {
    TL_PROFILER_PUSH
    // Windows-specific platform cleanup
    // (Currently no specific cleanup needed)
    TL_PROFILER_POP_WITH(true)
}

// ---------------------------------
// Windows Platform - Filesystem
// ---------------------------------

static i8 tl_winapi_filesystem_path_separator(void) {
    return '\\';
}

static const char* tl_winapi_filesystem_get_current_directory() {
    return _getcwd(NULL, 0);
}

// ---------------------------------
// Windows Platform - Timing
// ---------------------------------

static void tl_winapi_time_clock(TLDateTime* clock) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    clock->year = st.wYear;
    clock->month = (u8)st.wMonth;
    clock->day = (u8)st.wDay;
    clock->hour = (u8)st.wHour;
    clock->minute = (u8)st.wMinute;
    clock->second = (u8)st.wSecond;
    clock->millis = (u16)st.wMilliseconds;
}

static u64 tl_winapi_time_epoch_millis(void) {
    LARGE_INTEGER qpc;
    QueryPerformanceCounter(&qpc);
    // Shift 30 bits for millis (20 + 10, where 1024 ≈ 1000)
    return (qpc_epoch_offset >> 10) + ((qpc.QuadPart * qpc_to_micros_mul) >> 30);
}

static u64 tl_winapi_time_epoch_micros(void) {
    LARGE_INTEGER qpc;
    QueryPerformanceCounter(&qpc);
    return qpc_epoch_offset + ((qpc.QuadPart * qpc_to_micros_mul) >> qpc_to_micros_shift);
}

#endif // TL_PLATFORM_WINDOWS

#endif // __TELEIOS_PLATFORM_WINDOWS__