#include "libk/libk.h"

typedef struct {
    const char* name;
    u64 timestamp;
    u64 ticks;
} TLProfile;

static TLProfile profile[U8_MAX];

void k_runtime_profiler_push(const char *name) {
    u32 index = 0;
    for (u32 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) {
            index = i;
            break;
        }
    }

    profile[index].name = name;
    profile[index].timestamp = k_time_epoch_micros();
}

static u8 k_runtime_profiler_index(const char *name) {
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

    KFATAL("Profile [%s] not found", name)
}

u64 k_runtime_profiler_elapsed(const char *name) {
    const u8 index = k_runtime_profiler_index(name);
    return k_time_epoch_micros() - profile[index].timestamp;
}

void k_runtime_profiler_pop(const char *name) {
    const u8 index = k_runtime_profiler_index(name);
    memset(&profile[index], 0, sizeof(TLProfile));
}


#if ! defined(K_RUNTIME_FRAME_MAXIMUM)
#   define K_RUNTIME_FRAME_MAXIMUM 10
#endif

struct KFrame{
    u64 timestamp;
    char filename[K_RUNTIME_STRING_SIZE];
    char function[K_RUNTIME_STRING_SIZE];
    char arguments[K_RUNTIME_FRAME_ARGUMENTS_SIZE];
    u32 lineno;
};

static u8 frame_index = U8_MAX;
static u8 frame_max_depth = U8_MAX;
static KFrame frame[K_RUNTIME_FRAME_MAXIMUM];

void k_runtime_frame_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
    frame_index++;
    if (frame_index >= K_RUNTIME_FRAME_MAXIMUM) {
        KFATAL("stack_index exceeded")
    }

    if (frame_index > frame_max_depth) {
        frame_max_depth = frame_index;
    }

    frame[frame_index].lineno = lineno;
    frame[frame_index].timestamp = k_time_epoch_micros();
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u16 i = 0;
        for ( ; i <K_RUNTIME_STRING_SIZE ; ++i) {
            if (function[i] == '\0') break;
            frame[frame_index].function[i] = function[i];
        }

        memset(&frame[frame_index].function[i], 0,K_RUNTIME_STRING_SIZE - i);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    memset(frame[frame_index].arguments, 0, K_RUNTIME_FRAME_ARGUMENTS_SIZE);
    if (arguments != NULL){
        va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(frame[frame_index].arguments, K_RUNTIME_FRAME_ARGUMENTS_SIZE, arguments, arg_ptr);
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
        for ( ; i <K_RUNTIME_STRING_SIZE ; ++i) {
            if (filename[index + i] == '\0') break;
            frame[frame_index].filename[i] = filename[index + i];
        }

        memset(&frame[frame_index].filename[i], 0,K_RUNTIME_STRING_SIZE - i);
    }
}

void k_runtime_frame_pop() {
    memset(&frame[frame_index], 0 , sizeof(KFrame));
    frame_index--;
}