#include "core.h"
// ########################################################
//                    TIME FUNCTIONS
// ########################################################
#include <time.h>

#if defined(TLPLATFORM_LINUX)
#include <sys/time.h>
#endif

void tl_time_clock(TLClock* clock) {
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) return;

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

    clock->year = localtime.tm_year + 1900;
    clock->month = localtime.tm_mon + 1;
    clock->day = localtime.tm_mday;
    clock->hour = localtime.tm_hour;
    clock->minute = localtime.tm_min;
    clock->second = localtime.tm_sec;
    clock->millis = now.tv_nsec / 1000;
}

u64 tl_time_epoch_millis(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME_COARSE, &now);
    return (uint64_t) now.tv_sec * 1000000 + now.tv_nsec;
}

u64 tl_time_epoch_micros(void) {
    u64 micros = 0;
#if defined(TLPLATFORM_LINUX)
    struct timeval now = { 0 };
    gettimeofday(&now, NULL);
    micros = (uint64_t) now.tv_sec * 1000000 + now.tv_usec;
#elif defined(TLPLATFORM_WINDOWS)
    FILETIME ft; GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // FILETIME is in 100-nanosecond intervals since January 1, 1601 (UTC).
    // We need to convert it to microseconds since January 1, 1970 (UTC).

    // Subtract the number of 100-nanosecond intervals between the two dates.
    // The value is 116444736000000000 (obtained from various sources).
    micros = (uli.QuadPart - 116444736000000000ULL) / 10; // Convert to microseconds
#endif
    return micros;
}
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
        if (i++ == U32_MAX) { i = 0; }
    }

    TLClock clock; tl_time_clock(&clock);
    fprintf(stdout, "%s%d-%02d-%02d %02d:%02d:%02d,%06u %20s:%04d %s %s\n\033[1;30m",
        colors[level],
        clock.year, clock.month, clock.day,
        clock.hour, clock.minute, clock.second, clock.millis,
        filename + index, lineno,
        strings[level],
        output
    );

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
#endif

void tl_trace_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
#if ! defined(TELEIOS_BUILD_RELEASE)

    global->stack_index++;
    if (global->stack_index >= TL_STACK_SIZE_MAXIMUM) {
        TLFATAL("global->stack_index exceeded")
    }

    if (global->stack_index > global->stack_maximum) {
        global->stack_maximum = global->stack_index;
    }

    global->stack[global->stack_index].lineno = lineno;
    global->stack[global->stack_index].timestamp = tl_time_epoch_micros();
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u16 i = 0;
        for ( ; i < TL_STACK_STRING_SIZE ; ++i) {
            if (function[i] == '\0') break;
            global->stack[global->stack_index].function[i] = function[i];
        }

        tl_platform_memory_set(&global->stack[global->stack_index].function[i], 0, TL_STACK_STRING_SIZE - i);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    tl_platform_memory_set(global->stack[global->stack_index].arguments, 0, TL_STACK_ARGUMENTS_SIZE);
    if (arguments != NULL){
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(global->stack[global->stack_index].arguments, TL_STACK_ARGUMENTS_SIZE, arguments, arg_ptr);
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
        for ( ; i < TL_STACK_STRING_SIZE ; ++i) {
            if (filename[index + i] == '\0') break;
            global->stack[global->stack_index].filename[i] = filename[index + i];
        }

        tl_platform_memory_set(&global->stack[global->stack_index].filename[i], 0, TL_STACK_STRING_SIZE - i);
    }
#endif
}

void tl_trace_pop() {
#if ! defined(TELEIOS_BUILD_RELEASE)
    tl_platform_memory_set(&global->stack[global->stack_index], 0 , sizeof(TLStackFrame));
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
    profile[index].timestamp = tl_time_epoch_micros();
#endif
}

static u8 tl_profiler_index(const char *name) {
    for (u8 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) continue;

        b8 found = true;
        for (u64 j = 0; j < U64_MAX; ++j) {
            if (profile[i].name[j] != name[j]) {
                found = false;
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
    return tl_time_epoch_micros() - profile[index].timestamp;
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
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    tl_platform_memory_set(&profile[index], 0, sizeof(TLProfile));
#endif
}
// #####################################################################################################################
//
//                                                     EVENT
//
// #####################################################################################################################
static PFN_handler subscribers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };

static const char* tl_event_name(const TLEventCodes code) {
    TL_STACK_PUSHA("%d", code)
    switch (code) {
        default                             : {
                u16 digits = tl_number_i32_digits(code);
                char buffer[digits + 1]; tl_char_from_i32(buffer, code, 10);
                TL_STACK_POPV(buffer);
        }
        case TL_EVENT_WINDOW_CREATED        : TL_STACK_POPV("TL_EVENT_WINDOW_CREATED")
        case TL_EVENT_WINDOW_RESIZED        : TL_STACK_POPV("TL_EVENT_WINDOW_RESIZED")
        case TL_EVENT_WINDOW_CLOSED         : TL_STACK_POPV("TL_EVENT_WINDOW_CLOSED")
        case TL_EVENT_WINDOW_MOVED          : TL_STACK_POPV("TL_EVENT_WINDOW_MOVED")
        case TL_EVENT_WINDOW_MINIMIZED      : TL_STACK_POPV("TL_EVENT_WINDOW_MINIMIZED")
        case TL_EVENT_WINDOW_MAXIMIZED      : TL_STACK_POPV("TL_EVENT_WINDOW_MAXIMIZED")
        case TL_EVENT_WINDOW_RESTORED       : TL_STACK_POPV("TL_EVENT_WINDOW_RESTORED")
        case TL_EVENT_WINDOW_FOCUS_GAINED   : TL_STACK_POPV("TL_EVENT_WINDOW_FOCUS_GAINED")
        case TL_EVENT_WINDOW_FOCUS_LOST     : TL_STACK_POPV("TL_EVENT_WINDOW_FOCUS_LOST")
        case TL_EVENT_MAXIMUM               : TL_STACK_POPV("TL_EVENT_MAXIMUM")
    }
}

b8 tl_event_subscribe(const u16 event, const PFN_handler handler) {
    TL_STACK_PUSHA("%u, 0x%p", event, handler)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TL_STACK_POPV(false)
    }

    if (subscribers[event][U8_MAX - 1] != NULL) {
        TLWARN("Event %u reached maximum of %d handlers", event, U8_MAX - 1);
        TL_STACK_POPV(false)
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] == NULL) {
            TLTRACE("Subscribing to %s handler function 0x%p", tl_event_name(event), handler)
            TLTRACE("Subscribing to %s has %d handlers", tl_event_name(event), i + 1)
            subscribers[event][i] = handler;
            break;
        }
    }

    TL_STACK_POPV(true)
}

void tl_event_submit(const u16 event, const TLEvent* data) {
    TL_STACK_PUSHA("%u, 0x%p", event, data)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Event type beyond %d", TL_EVENT_MAXIMUM);
        TL_STACK_POP
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] != NULL) {
            const TLEventStatus status = (*subscribers[event][i])(data);
            if (status == TL_EVENT_CONSUMED) {
                break;
            }
        }
    }

    TL_STACK_POP
}
// #####################################################################################################################
//
//                                                     GRAPHICS
//
// #####################################################################################################################
b8 tl_graphics_initialize(void) {
    TL_STACK_PUSH

    TLTRACE("Initializing Graphics Engine");
    glfwMakeContextCurrent(global->platform.window.handle);

    TLDEBUG("CGLM_VERSION %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("Failed to initialize GLAD")
        TL_STACK_POPV(false)
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

    TL_STACK_POPV(true)
}

b8 tl_graphics_terminate(void) {
    TL_STACK_PUSH
    TLTRACE("Terminating Graphics Engine");
    tl_platform_memory_set(&global->platform.graphics, 0, sizeof(global->platform.graphics));
    glfwMakeContextCurrent(NULL);
    TL_STACK_POPV(true)
}
// #####################################################################################################################
//
//                                                     NUMBER
//
// #####################################################################################################################
TL_INLINE u16 tl_number_i32_digits(const i32 number) {
    TL_STACK_PUSHA("%d", number)
    u16 digits = 0;
    i32 desired = number;

    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);
    TL_STACK_POPV(digits)
}
// #####################################################################################################################
//
//                                                     STRING
//
// #####################################################################################################################
TL_INLINE u32 tl_char_length(const char *string) {
    TL_STACK_PUSHA("0x%p", string)
    if (string == NULL ) TL_STACK_POPV(U32_MAX);
    if (*string == '\0') TL_STACK_POPV(0);

    u32 index = 0;

    const char* character = string;
    while (*character++ != '\0') {
        if (index++ == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TL_STACK_POPV(index)
}

TL_INLINE u32 tl_char_last_index_of(const char *string, const char character) {
    TL_STACK_PUSHA("0x%p, %c", string, character)
    u32 index = 0;
    const char* s = string;
    for (u32 i = 0; *s != '\0' ; ++s) {
        if (*s == character) {
            index = i;
        }

        if (++i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TL_STACK_POPV(index)
}

TL_INLINE u32 tl_char_index_of(const char *string, const char token) {
    TL_STACK_PUSHA("0x%p, %c", string, token)
    const char* s = string;
    for (u32 i = 0; *s != '\0' ; ++s) {
        if (*s == token) {
            TL_STACK_POPV(i)
        }

        if (++i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TL_STACK_POPV(U32_MAX)
}

b8 tl_char_equals(const char *string, const char *guess) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TL_STACK_POPV(false)
    }

    const u64 length = tl_char_length(string);
    if (tl_char_length(guess) != length) {
        TL_STACK_POPV(false)
    }

    for (u64 i = 0; i < length; ++i) {
        if (string[i] != guess[i]) {
            TL_STACK_POPV(false)
        }
    }

    TL_STACK_POPV(true)
}

b8 tl_char_start_with(const char *string, const char *guess) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TL_STACK_POPV(false)
    }

    const u64 length = tl_char_length(guess);
    if (length > tl_char_length(string)) {
        TL_STACK_POPV(false)
    }

    for (u64 i = 0; i < length; ++i) {
        if (string[i] != guess[i]) {
            TL_STACK_POPV(false)
        }
    }

    TL_STACK_POPV(true)
}

u32 tl_char_copy(char *target, const char *source, const u32 length) {
    TL_STACK_PUSHA("0x%p, 0x%p", target, source)

    u32 copied = 0;
    for (u32 i = 0; i < length ; ++i) {
        target[i] = source[i];
        copied++;
    }

    TL_STACK_POPV(copied)
}

TL_INLINE void tl_char_join(char *buffer, const u64 size, const char *str0, const char *str1) {
    TL_STACK_PUSHA("0x%p, %d, %s, %s", buffer, size, str0, str1)
    if (tl_char_length(str0) + tl_char_length(str1) > size) TL_STACK_POP
    sprintf(buffer, "%s%s", str0, str1);
    TL_STACK_POP
}

TL_INLINE void tl_char_from_i32(char *buffer, i32 value, const u8 base) {
    TL_STACK_PUSHA("0x%p, 0x%p, %d, %d", buffer, value, base)

    // check that the base if valid
    if (base < 2 || base > 36) { TL_STACK_POP }

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

    TL_STACK_POP
}
// #####################################################################################################################
//
//                                                     PLATFORM
//
// #####################################################################################################################
TL_INLINE void* tl_platform_memory_alloc(const u64 size) {
    return malloc(size);
}

TL_INLINE void tl_platform_memory_free(void *block) {
    free(block);
}
TL_INLINE void tl_platform_memory_set(void *block, const i32 value, const u64 size) {
    memset(block, value, size);
}

TL_INLINE void tl_platform_memory_copy(void *target, const void *source, const u64 size) {
    memcpy(target, source, size);
}

b8 tl_platform_initialize(void) {
    TL_STACK_PUSH
    TLDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)

    TLTRACE("Initializing GLFW");
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        TL_STACK_POPV(false)
    }

    TLDEBUG("Platform initialized in %llu micros", TL_PROFILER_MICROS);
    TL_STACK_POPV(true)
}

b8 tl_platform_terminate(void) {
    TL_STACK_PUSH

    glfwTerminate();
    TL_STACK_POPV(true)
}