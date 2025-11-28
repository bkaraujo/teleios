#ifndef __TELEIOS_PLATFORM_LINUX__
#define __TELEIOS_PLATFORM_LINUX__

#include "teleios/defines.h"

#ifdef TL_PLATFORM_LINUX

#include "teleios/teleios.h"
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

// ---------------------------------
// Linux Platform - Initialization
// ---------------------------------

static b8 tl_lnx_initialize(void) {
    TL_PROFILER_PUSH
    // Linux-specific platform initialization
    // (Currently no specific setup needed)
    TL_PROFILER_POP_WITH(true)
}

static b8 tl_lnx_terminate(void) {
    TL_PROFILER_PUSH
    // Linux-specific platform cleanup
    // (Currently no specific cleanup needed)
    TL_PROFILER_POP_WITH(true)
}

// ---------------------------------
// Linux Platform - Filesystem
// ---------------------------------

static i8 tl_lnx_filesystem_path_separator(void) {
    return '/';
}

static const char* tl_lnx_filesystem_get_current_directory(void) {
    return getcwd(NULL, 0);
}

static b8 tl_lnx_filesystem_exists(const TLString* path) {
    const char* cpath = tl_string_cstr(path);
    struct stat st;
    return (stat(cpath, &st) == 0 && S_ISREG(st.st_mode));
}

static u64 tl_lnx_filesystem_size(const TLString* path) {
    const char* cpath = tl_string_cstr(path);
    struct stat st;
    if (stat(cpath, &st) != 0) return 0;
    return (u64)st.st_size;
}

static TLString* tl_lnx_filesystem_read(const TLString* path) {
    const char* cpath = tl_string_cstr(path);
    FILE* file = fopen(cpath, "rb");
    if (!file) {
        TLERROR("Failed to open file: %s", cpath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length < 0) {
        TLERROR("Failed to get file size: %s", cpath);
        fclose(file);
        return NULL;
    }

    char* buffer = malloc(length + 1);
    if (!buffer) {
        TLERROR("Failed to allocate memory for file: %s", cpath);
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, length, file);
    buffer[read_size] = '\0';
    fclose(file);

    TLString* str = tl_string_create(global->allocator, buffer);
    free(buffer);
    return str;
}

// ---------------------------------
// Linux Platform - Timing
// ---------------------------------

static void tl_lnx_time_clock(TLDateTime* clock) {
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) return;

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

    clock->year = localtime.tm_year + 1900;
    clock->month = (u8)(localtime.tm_mon + 1);
    clock->day = (u8)localtime.tm_mday;
    clock->hour = (u8)localtime.tm_hour;
    clock->minute = (u8)localtime.tm_min;
    clock->second = (u8)localtime.tm_sec;
    clock->millis = (u16)(now.tv_nsec / 1000000);
}

static u64 tl_lnx_time_epoch_millis(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME_COARSE, &now);  // ~20-30 ns
    return (u64)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

static u64 tl_lnx_time_epoch_micros(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME, &now);  // ~25-50 ns
    return (u64)now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

#endif // TL_PLATFORM_LINUX

#endif // __TELEIOS_PLATFORM_LINUX__