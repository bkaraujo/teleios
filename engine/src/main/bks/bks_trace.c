#include "bks/bks.h"

#if ! defined(BKS_FRAME_MAXIMUM)
#   define BKS_FRAME_MAXIMUM 10
#endif

typedef struct {
  u64 timestamp;
  char filename[100];
  char function[100];
  char arguments[1024];
  u32 lineno;
} BKSStackFrame;

static u8 stack_index = U8_MAX;
static u8 stack_maximum = U8_MAX;
static BKSStackFrame stack[BKS_FRAME_MAXIMUM];

void bks_trace_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
    stack_index++;
    if (stack_index >= BKS_STACK_SIZE_MAXIMUM) {
        BKSFATAL("stack_index exceeded")
    }

    if (stack_index > stack_maximum) {
        stack_maximum = stack_index;
    }

    stack[stack_index].lineno = lineno;
    stack[stack_index].timestamp = bks_time_epoch_micros();
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u16 i = 0;
        for ( ; i < BKS_STACK_STRING_SIZE ; ++i) {
            if (function[i] == '\0') break;
            stack[stack_index].function[i] = function[i];
        }

        __builtin_memset(&stack[stack_index].function[i], 0, BKS_STACK_STRING_SIZE - i);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    __builtin_memset(stack[stack_index].arguments, 0, BKS_STACK_ARGUMENTS_SIZE);
    if (arguments != NULL){
        __builtin_va_list arg_ptr;
        __builtin_va_start(arg_ptr, arguments);
        __builtin_vsnprintf(stack[stack_index].arguments, BKS_STACK_ARGUMENTS_SIZE, arguments, arg_ptr);
        __builtin_va_end(arg_ptr);
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
        for ( ; i < BKS_STACK_STRING_SIZE ; ++i) {
            if (filename[index + i] == '\0') break;
            stack[stack_index].filename[i] = filename[index + i];
        }

        __builtin_memset(&stack[stack_index].filename[i], 0, BKS_STACK_STRING_SIZE - i);
    }
}

void bks_trace_pop() {
    __builtin_memset(&stack[stack_index], 0 , sizeof(BKSStackFrame));
    stack_index--;
}

typedef struct {
    const char* name;
    u64 timestamp;
    u64 ticks;
} TLProfile;

static TLProfile profile[U8_MAX];

void bks_profiler_begin(const char *name) {
    u32 index = 0;
    for (u32 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) {
            index = i;
            break;
        }
    }

    profile[index].name = name;
    profile[index].timestamp = bks_time_epoch_micros();
}

static u8 bks_profiler_index(const char *name) {
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

    BKSFATAL("Profile [%s] not found", name)
}

u64 bks_profiler_time(const char *name) {
    const u8 index = bks_profiler_index(name);
    return bks_time_epoch_micros() - profile[index].timestamp;
}

void bks_profiler_tick(const char *name) {
    const u8 index = bks_profiler_index(name);
    profile[index].ticks++;
}

u64 bks_profiler_ticks(const char *name) {
    const u8 index = bks_profiler_index(name);
    return profile[index].ticks;
}

void bks_profiler_end(const char *name) {
    const u8 index = bks_profiler_index(name);
    __builtin_memset(&profile[index], 0, sizeof(TLProfile));
}
