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
    LARGE_INTEGER freq; QueryPerformanceFrequency(&freq);
    qpc_freq = freq.QuadPart;

    // Pre-calculate multiplier to avoid division in hot path
    // Using 20-bit shift for precision
    qpc_to_micros_shift = 20;
    qpc_to_micros_mul = (1000000ULL << qpc_to_micros_shift) / qpc_freq;

    // Calculate offset to Unix epoch
    FILETIME ft; GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    const u64 epoch_micros = (uli.QuadPart - 116444736000000000ULL) / 10;

    LARGE_INTEGER qpc_now; QueryPerformanceCounter(&qpc_now);
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

static const char* tl_winapi_filesystem_get_current_directory(void) {
    return _getcwd(NULL, 0);
}

static b8 tl_winapi_filesystem_exists(const TLString* path) {
    const char* cpath = tl_string_cstr(path);
    DWORD attrib = GetFileAttributesA(cpath);
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

static u64 tl_winapi_filesystem_size(const TLString* path) {
    const char* cpath = tl_string_cstr(path);
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(cpath, GetFileExInfoStandard, &fad))
        return 0;
    
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}

static TLString* tl_winapi_filesystem_read(const TLString* path) {
    const char* cpath = tl_string_cstr(path);
    HANDLE hFile = CreateFileA(cpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        TLERROR("Failed to open file: %s", cpath);
        return NULL;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        TLERROR("Failed to get file size: %s", cpath);
        CloseHandle(hFile);
        return NULL;
    }

    // Allocate buffer using malloc for raw read, then create TLString
    // We use malloc here because we need a raw buffer to read into.
    // TLString will copy it into its own managed memory using global->allocator.
    char* buffer = (char*)tl_memory_alloc(global->allocator, TL_MEMORY_BLOCK, size.QuadPart + 1);
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, (DWORD)size.QuadPart, &bytesRead, NULL)) {
        TLERROR("Failed to read file: %s", cpath);
        tl_memory_free(global->allocator, buffer);
        CloseHandle(hFile);
        return NULL;
    }

    buffer[bytesRead] = '\0';
    CloseHandle(hFile);

    TLString* str = tl_string_create(global->allocator, buffer);
    tl_memory_free(global->allocator, buffer);
    return str;
}

// ---------------------------------
// Windows Platform - Timing
// ---------------------------------

static void tl_winapi_time_clock(TLDateTime* clock) {
    SYSTEMTIME st; GetLocalTime(&st);

    clock->year = st.wYear;
    clock->month = (u8)st.wMonth;
    clock->day = (u8)st.wDay;
    clock->hour = (u8)st.wHour;
    clock->minute = (u8)st.wMinute;
    clock->second = (u8)st.wSecond;
    clock->millis = (u16)st.wMilliseconds;
}

static u64 tl_winapi_time_epoch_millis(void) {
    LARGE_INTEGER qpc; QueryPerformanceCounter(&qpc);
    // Shift 30 bits for millis (20 + 10, where 1024 ≈ 1000)
    return (qpc_epoch_offset >> 10) + ((qpc.QuadPart * qpc_to_micros_mul) >> 30);
}

static u64 tl_winapi_time_epoch_micros(void) {
    LARGE_INTEGER qpc; QueryPerformanceCounter(&qpc);
    return qpc_epoch_offset + ((qpc.QuadPart * qpc_to_micros_mul) >> qpc_to_micros_shift);
}

#endif // TL_PLATFORM_WINDOWS

#endif // __TELEIOS_PLATFORM_WINDOWS__