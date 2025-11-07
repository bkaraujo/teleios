#ifndef __TELEIOS_PLATFORM_WINDOWS__
#define __TELEIOS_PLATFORM_WINDOWS__
#include "teleios/defines.h"
#ifdef TL_PLATFORM_WINDOWS
#include "teleios/teleios.h"

static u64 qpc_freq = 0;
static u64 qpc_to_micros_mul = 0;  // Pre-calculado
static u64 qpc_to_micros_shift = 0;
static u64 qpc_epoch_offset = 0;

b8 tl_winapi_initialize(void) {
    tl_profiler_frame_push("platform.c", 19, "tl_platform_initialize", NULL);

    TL_PROFILER_PUSH
    LARGE_INTEGER freq; QueryPerformanceFrequency(&freq);
    qpc_freq = freq.QuadPart;
    
    // Pre-calcula multiplicador para evitar divisão
    // Usando shift de 20 bits para precisão
    qpc_to_micros_shift = 20;
    qpc_to_micros_mul = (1000000ULL << qpc_to_micros_shift) / qpc_freq;
    
    // Calcula offset
    FILETIME ft; GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    u64 epoch_micros = (uli.QuadPart - 116444736000000000ULL) / 10;
    
    LARGE_INTEGER qpc_now; QueryPerformanceCounter(&qpc_now);
    qpc_epoch_offset = epoch_micros - ((qpc_now.QuadPart * qpc_to_micros_mul) >> qpc_to_micros_shift);
    TL_PROFILER_POP_WITH(true)
}

i8 tl_winapi_filesystem_path_separator(void) {
    return '\\';
}

void tl_winapi_time_clock(TLDateTime* clock) {
    SYSTEMTIME st; GetLocalTime(&st);
    
    clock->year = st.wYear;
    clock->month = (u8)st.wMonth;
    clock->day = (u8)st.wDay;
    clock->hour = (u8)st.wHour;
    clock->minute = (u8)st.wMinute;
    clock->second = (u8)st.wSecond;
    clock->millis = st.wMilliseconds;
}

u64 tl_winapi_time_epoch_millis(void) {
    LARGE_INTEGER qpc; QueryPerformanceCounter(&qpc);
    // Shift de 30 para millis (20 + 10, onde 1024 ≈ 1000)
    return (qpc_epoch_offset >> 10) + ((qpc.QuadPart * qpc_to_micros_mul) >> 30);
}

u64 tl_winapi_time_epoch_micros(void) {
    LARGE_INTEGER qpc; QueryPerformanceCounter(&qpc);
    return qpc_epoch_offset + ((qpc.QuadPart * qpc_to_micros_mul) >> qpc_to_micros_shift);
}

b8 tl_winapi_terminate(void) {
    TL_PROFILER_PUSH
    
    TL_PROFILER_POP_WITH(true)
}

#endif
#endif