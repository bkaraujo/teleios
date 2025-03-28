#include "core.h"
// #####################################################################################################################
//
//                                                     LOGGER
//
// #####################################################################################################################
#if ! defined(TELEIOS_BUILD_RELEASE)
#include "teleios/globals.h"
#endif

#if defined(TLPLATFORM_WINDOWS)
#   define ANSI_COLOR_FATAL   ""
#   define ANSI_COLOR_ERROR   ""
#   define ANSI_COLOR_WARN    ""
#   define ANSI_COLOR_INFO    ""
#   define ANSI_COLOR_DEBUG   ""
#   define ANSI_COLOR_TRACE   ""
#   define ANSI_COLOR_VERBOSE ""
#endif

#if defined(TLPLATFORM_LINUX)
#   include <time.h>
#   define ANSI_COLOR_FATAL   "\033[1;31m"
#   define ANSI_COLOR_ERROR   "\033[1;31m"
#   define ANSI_COLOR_WARN    "\033[1;33m"
#   define ANSI_COLOR_INFO    "\033[1;32m"
#   define ANSI_COLOR_DEBUG   "\033[1;34m"
#   define ANSI_COLOR_TRACE   "\033[1;36m"
#   define ANSI_COLOR_VERBOSE "\033[1;37m"
#endif

#if defined(TELEIOS_BUILD_RELEASE)
static TLLogLevel m_level = TL_LOG_LEVEL_INFO;
#else
static TLLogLevel m_level = TL_LOG_LEVEL_VERBOSE;
#endif
static const char *strings[] = {"VERBOSE ", "TRACE  ", "DEBUG  ", "INFO   ", "WARN   ", "ERROR  ", "FATAL  "};
static const char *colors[] = { ANSI_COLOR_VERBOSE , ANSI_COLOR_TRACE, ANSI_COLOR_DEBUG, ANSI_COLOR_INFO, ANSI_COLOR_WARN, ANSI_COLOR_ERROR, ANSI_COLOR_FATAL };

void tl_logger_loglevel(const TLLogLevel desired){
    m_level = desired;
}

void tl_logger_write(const TLLogLevel level, const char *filename, const u32 lineno, const char *message, ...) {
    if (level < m_level) { return; }

    char output[2048] = { 0 };
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(output, 2048, message, arg_ptr);
    va_end(arg_ptr);

    u32 index = 0;
    const char* character = filename;
    for (u16 i = 0; *character != '\0' ; ++character) {
        if (*character == '/') { index = i + 1; }
        if (++i == U32_MAX) { i = 0; }
    }

#ifdef TLPLATFORM_LINUX
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) return;

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

    fprintf(stdout, "%s%d-%02d-%02d %02d:%02d:%02d,%06ld %20s:%04d %s %s\n\033[1;30m",
        colors[level],
        localtime.tm_year + 1900, localtime.tm_mon + 1, localtime.tm_mday,
        localtime.tm_hour, localtime.tm_min, localtime.tm_sec, now.tv_nsec / 1000,
        filename + index, lineno,
        strings[level],
        output
    );
#endif

    fflush(stdout);
#if ! defined(TELEIOS_BUILD_RELEASE)
    // Print the stack trace
    if (level == TL_LOG_LEVEL_FATAL) {
        const char *format = "%66s at %20s:%04d %s(%s)\n\033[1;30m";

        fprintf(stdout, "\n");
        for (u8 i = global->stack_index ; i > 0 ; --i) {
            fprintf(stdout, format, colors[level], global->stack[i].filename, global->stack[i].lineno, global->stack[i].function, global->stack[i].arguments);
        }

        fprintf(stdout, format, colors[level], global->stack[0].filename, global->stack[0].lineno, global->stack[0].function, global->stack[0].arguments);
    }
#endif
}
// #####################################################################################################################
//
//                                                     META
//
// #####################################################################################################################
#if ! defined(TELEIOS_BUILD_RELEASE)
#   include <stdio.h>
#   if defined(TLPLATFORM_LINUX)
#        include <time.h>
#       include <sys/time.h>
#   endif
#endif

void tl_trace_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    if (global->stack_index >= sizeof(global->stack) / sizeof(TLStackFrame)) {
        TLFATAL("global->stack_index exceeded")
    }

    global->stack[global->stack_index].lineno = lineno;

#if defined(TLPLATFORM_LINUX)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    const u64 micros = (uint64_t) tv.tv_sec * 1000000 + tv.tv_usec;

    global->stack[global->stack_index].timestamp =  micros;
#elif defined(TLPLATFORM_WINDOWS)
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

    global->stack[global->stack_index].timestamp = epoch_microseconds
#endif

    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u16 i = 0;
        for ( ; i < STACK_SIZE_MAXIMUM ; ++i) {
            if (function[i] == '\0') break;
            global->stack[global->stack_index].function[i] = function[i];
        }

        tl_platform_memory_set(&global->stack[global->stack_index].function[i], 0, STACK_SIZE_MAXIMUM - i);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    tl_platform_memory_set(global->stack[global->stack_index].arguments, 0, 1024);
    if (arguments != NULL){
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(global->stack[global->stack_index].arguments, 1024, arguments, arg_ptr);
        va_end(arg_ptr);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u32 index = 0;
        const char* character = filename;
        for (u32 i = 0; *character != '\0' ; ++character) {
            if (*character == '/') { index = i + 1; }
            if (++i == U32_MAX) { i = 0; }
        }

        u16 i = 0;
        for ( ; i < STACK_SIZE_MAXIMUM ; ++i) {
            if (filename[index + i] == '\0') break;
            global->stack[global->stack_index].filename[i] = filename[index + i];
        }

        tl_platform_memory_set(&global->stack[global->stack_index].filename[i], 0, STACK_SIZE_MAXIMUM - i);
    }

    global->stack_index++;
    if (global->stack_index > global->stack_maximum) {
        global->stack_maximum = global->stack_index;
    }

    // TLVERBOSE("STACK PUSH :: %s:%d %s(%s)",
    //     global->stack[global->stack_index].filename, global->stack[global->stack_index].lineno,
    //     global->stack[global->stack_index].function, global->stack[global->stack_index].arguments
    // )
#endif
}

void tl_trace_pop() {
#if ! defined(TELEIOS_BUILD_RELEASE)
    if (global->stack_index == 0) TLWARN("global->stack_index is zero");

    // TLVERBOSE("STACK POP :: %s:%d %s(%s)",
    //     global->stack[global->stack_index].filename, global->stack[global->stack_index].lineno,
    //     global->stack[global->stack_index].function, global->stack[global->stack_index].arguments
    // )

    global->stack_index--;
#endif
}

typedef struct {
    const char* name;
    u64 timestamp;
    u64 ticks;
} TLProfile;

static TLProfile profile[U8_MAX];

void tl_profiler_begin(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    u32 index = 0;
    for (u32 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) {
            index = i;
            break;
        }
    }

    profile[index].name = name;
#if defined(TLPLATFORM_LINUX)
    struct timeval now; gettimeofday(&now, NULL);
    profile[index].timestamp = (u64) now.tv_sec * 1000000 + now.tv_usec;
#elif defined(TLPLATFORM_WINDOWS)
#erro "!!!"
#endif
#endif
}

static u8 tl_profiler_index(const char *name) {
    for (u8 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) continue;

        b8 found = TRUE;
        for (u64 j = 0; j < U64_MAX; ++j) {
            if (profile[i].name[j] != name[j]) {
                found = FALSE;
                break;
            }

            if (name[j] == '\0') break;
        }

        if (found) return i;
    }

    TLFATAL("Profile [%s] not found", name)
}

u64 tl_profiler_time(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
#if defined(TLPLATFORM_LINUX)
    struct timeval now = { 0 };
    gettimeofday(&now, NULL);
    return (uint64_t) now.tv_sec * 1000000 + now.tv_usec - profile[index].timestamp;
#elif defined(TLPLATFORM_WINDOWS)
#erro "!!!"
#else
    return U64_MAX;
#endif
#endif
}

void tl_profiler_tick(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    profile[index].ticks++;
#endif
}

u64 tl_profiler_ticks(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    return profile[index].ticks;
#else
    return U64_MAX;
#endif
}

void tl_profiler_end(const char *name) {
    const u8 index = tl_profiler_index(name);
    tl_platform_memory_set(&profile[index], 0, sizeof(TLProfile));
}
// #####################################################################################################################
//
//                                                     PLATFORM
//
// #####################################################################################################################



b8 tl_platform_initialize(void) {
    TLSTACKPUSH

    TLDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)
    // --------------------------------------------------------------------------------------
    // Initialize GLFW
    // --------------------------------------------------------------------------------------
    TLTRACE("Initializing GLFW");
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        TLSTACKPOPV(FALSE)
    }


    TLDEBUG("Platform initialized in %llu micros", TLPROFILER_MICROS);
    TLSTACKPOPV(TRUE)
}

b8 tl_platform_terminate(void) {
    TLSTACKPUSH

    glfwTerminate();

    TLSTACKPOPV(TRUE)
}

#if defined(TLPLATFORM_LINUX)
// ########################################################
//                    MEMORY FUNCTIONS
// ########################################################
TLINLINE void* tl_platform_memory_alloc(const u64 size) {
    return malloc(size);
}

TLINLINE void tl_platform_memory_free(void *block) {
    free(block);
}
TLINLINE void tl_platform_memory_set(void *block, const i32 value, const u64 size) {
    memset(block, value, size);
}

TLINLINE void tl_platform_memory_copy(void *target, const void *source, const u64 size) {
    memcpy(target, source, size);
}

// ########################################################
//                    TIME FUNCTIONS
// ########################################################
void tl_time_clock(TLClock* clock) {
    TLSTACKPUSHA("0x%p", clock)
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) TLSTACKPOP

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) TLSTACKPOP

    clock->year = localtime.tm_year + 1900;
    clock->month = localtime.tm_mon + 1;
    clock->day = localtime.tm_mday;
    clock->hour = localtime.tm_hour;
    clock->minute = localtime.tm_min;
    clock->second = localtime.tm_sec;
    clock->millis = now.tv_nsec / 1000;

    TLSTACKPOP
}

u64 tl_time_epoch_millis(void) {
    TLSTACKPUSH
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME_COARSE, &now);
    const u64 micros = (uint64_t) now.tv_sec * 1000000 + now.tv_nsec;
    TLSTACKPOPV(micros);
}

u64 tl_time_epoch_micros(void) {
    TLSTACKPUSH
    struct timeval now = { 0 };
    gettimeofday(&now, NULL);
    const u64 micros = (uint64_t) now.tv_sec * 1000000 + now.tv_usec;
    TLSTACKPOPV(micros);
}
#elif defined(TLPLATFORM_WINDOWS)
#include "teleios/core.h"

// ########################################################
//                    TIME FUNCTIONS
// ########################################################
u64 tl_time_epoch(void) {
    TLSTACKPUSH

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

    TLSTACKPOPV(epoch_microseconds)
}

#endif
// #####################################################################################################################
//
//                                                     EVENT
//
// #####################################################################################################################
static PFN_handler subscribers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };

static const char* tl_event_name(const TLEventCodes code) {
    TLSTACKPUSHA("%d", code)
    switch (code) {
        default                             : {
                u16 digits = tl_number_i32_digits(code);
                char buffer[digits + 1]; tl_char_from_i32(buffer, code, 10);
                TLSTACKPOPV(buffer);
        }
        case TL_EVENT_WINDOW_CREATED        : TLSTACKPOPV("TL_EVENT_WINDOW_CREATED")
        case TL_EVENT_WINDOW_RESIZED        : TLSTACKPOPV("TL_EVENT_WINDOW_RESIZED")
        case TL_EVENT_WINDOW_CLOSED         : TLSTACKPOPV("TL_EVENT_WINDOW_CLOSED")
        case TL_EVENT_WINDOW_MOVED          : TLSTACKPOPV("TL_EVENT_WINDOW_MOVED")
        case TL_EVENT_WINDOW_MINIMIZED      : TLSTACKPOPV("TL_EVENT_WINDOW_MINIMIZED")
        case TL_EVENT_WINDOW_MAXIMIZED      : TLSTACKPOPV("TL_EVENT_WINDOW_MAXIMIZED")
        case TL_EVENT_WINDOW_RESTORED       : TLSTACKPOPV("TL_EVENT_WINDOW_RESTORED")
        case TL_EVENT_WINDOW_FOCUS_GAINED   : TLSTACKPOPV("TL_EVENT_WINDOW_FOCUS_GAINED")
        case TL_EVENT_WINDOW_FOCUS_LOST     : TLSTACKPOPV("TL_EVENT_WINDOW_FOCUS_LOST")
        case TL_EVENT_MAXIMUM               : TLSTACKPOPV("TL_EVENT_MAXIMUM")
    }
}

b8 tl_event_subscribe(const u16 event, const PFN_handler handler) {
    TLSTACKPUSHA("%u, 0x%p", event, handler)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TLSTACKPOPV(FALSE)
    }

    if (subscribers[event][U8_MAX - 1] != NULL) {
        TLWARN("Event %u reached maximum of %d handlers", event, U8_MAX - 1);
        TLSTACKPOPV(FALSE)
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] == NULL) {
            TLTRACE("Subscribing to %s handler function 0x%p", tl_event_name(event), handler)
            TLTRACE("Subscribing to %s has %d handlers", tl_event_name(event), i + 1)
            subscribers[event][i] = handler;
            break;
        }
    }

    TLSTACKPOPV(TRUE)
}

void tl_event_submit(const u16 event, const TLEvent* data) {
    TLSTACKPUSHA("%u, 0x%p", event, data)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TLSTACKPOP
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] != NULL) {
            const TLEventStatus status = (*subscribers[event][i])(data);
            if (status == TL_EVENT_CONSUMED) {
                break;
            }
        }
    }

    TLSTACKPOP
}
// #####################################################################################################################
//
//                                                     GRAPHICS
//
// #####################################################################################################################
b8 tl_graphics_initialize(void) {
    TLSTACKPUSH
    glfwMakeContextCurrent(global->platform.window.handle);

    TLDEBUG("CGLM_VERSION %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("Failed to initialize GLAD")
        TLSTACKPOPV(FALSE)
    }

    TLDEBUG("GL_VERSION %s", glGetString(GL_VERSION))

    if (global->platform.graphics.vsync) {
        TLDEBUG("vsync: on")
        glfwSwapInterval(1);
    } else {
        TLDEBUG("vsync: off")
        glfwSwapInterval(0);
    }

    if (global->platform.graphics.wireframe) {
        TLDEBUG("wireframe: on")
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        TLDEBUG("wireframe: off")
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    TLSTACKPOPV(TRUE)
}

b8 tl_graphics_terminate(void) {
    TLSTACKPUSH
    tl_platform_memory_set(&global->platform.graphics, 0, sizeof(global->platform.graphics));
    TLSTACKPOPV(TRUE)
}
// #####################################################################################################################
//
//                                                     NUMBER
//
// #####################################################################################################################
TLINLINE u16 tl_number_i32_digits(const i32 number) {
    TLSTACKPUSHA("%d", number)
    u16 digits = 0;
    i32 desired = number;
    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);

    TLSTACKPOPV(digits)
}
// #####################################################################################################################
//
//                                                     STRING
//
// #####################################################################################################################

TLINLINE u32 tl_char_length(const char *string) {
    TLSTACKPUSHA("0x%p", string)
    if (string == NULL ) TLSTACKPOPV(U32_MAX);
    if (*string == '\0') TLSTACKPOPV(0);

    u32 index = 0;

    const char* character = string;
    while (*character++ != '\0') {
        if (index++ == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TLSTACKPOPV(index)
}

TLINLINE u32 tl_char_last_index(const char *string, const char token) {
    TLSTACKPUSHA("0x%p, %c", string, token)
    u32 index = 0;
    const char* s = string;
    for (u32 i = 0; *s != '\0' ; ++s) {
        if (*s == token) {
            index = i + 1;
        }

        i++;
        if (i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TLSTACKPOPV(index)
}

TLINLINE u32 tl_char_index_of(const char *string, const char token) {
    TLSTACKPUSHA("0x%p, %c", string, token)
    const char* s = string;
    for (u32 i = 0; *s != '\0' ; ++s) {
        if (*s == token) {
            TLSTACKPOPV(i)
        }

        i++;
        if (i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TLSTACKPOPV(U32_MAX)
}

b8 tl_char_equals(const char *string, const char *guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TLSTACKPOPV(FALSE)
    }

    const u64 length = tl_char_length(string);
    if (tl_char_length(guess) != length) {
        TLSTACKPOPV(FALSE)
    }

    for (u64 i = 0; i < length; i++) {
        if (string[i] != guess[i]) {
            TLSTACKPOPV(FALSE)
        }
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_char_start_with(const char *string, const char *guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TLSTACKPOPV(FALSE)
    }

    const u64 length = tl_char_length(guess);
    if (length > tl_char_length(string)) {
        TLSTACKPOPV(FALSE)
    }

    for (u64 i = 0; i < length; i++) {
        if (string[i] != guess[i]) {
            TLSTACKPOPV(FALSE)
        }
    }

    TLSTACKPOPV(TRUE)
}

u32 tl_char_copy(char *target, const char *source) {
    TLSTACKPUSHA("0x%p, 0x%p", target, source)
    const u32 target_length = tl_char_length(target);
    if (target_length == 0 || target_length == U32_MAX) {
        TLSTACKPOPV(0)
    }

    const u32 source_length = tl_char_length(source);
    if (source_length == 0 || source_length == U32_MAX) {
        TLSTACKPOPV(0)
    }

    const char *s = source;
    char *t = target;

    u32 copied = 0;
    while (*s != '\0' && *t != '\0') {
        *t = *s;
        s++; t++;
        copied++;
    }

    TLSTACKPOPV(copied)
}

TLINLINE void tl_char_join(char *buffer, const u64 size, const char *str0, const char *str1) {
    TLSTACKPUSHA("0x%p, %d, %s, %s", buffer, size, str0, str1)
    if (tl_char_length(str0) + tl_char_length(str1) > size) TLSTACKPOP
    sprintf(buffer, "%s%s", str0, str1);
    TLSTACKPOP
}

TLINLINE void tl_char_from_i32(char *buffer, i32 value, const u8 base) {
    TLSTACKPUSHA("0x%p, 0x%p, %d, %d", buffer, value, base)

    // check that the base if valid
    if (base < 2 || base > 36) { TLSTACKPOP }

    int tmp_value;
    char *ptr  = buffer;
    char *ptr1 = buffer;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';

    // Reverse the string
    while(ptr1 < ptr) {
        const char tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }

    TLSTACKPOP
}

// #####################################################################################################################
//
//                                                     LIFECYCLE
//
// #####################################################################################################################
b8 tl_core_initialize(void) {
    return TRUE;
}

b8 tl_core_terminate(void) {
    return TRUE;
}
