#include "teleios/core.h"
#include "teleios/core/meta.h"

#include <stdio.h>
#include <stdarg.h>

void tl_meta_frame_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
#ifndef TELEIOS_BUILD_RELEASE
    if (runtime->stack_size >= U8_MAX) {
        TLFATAL("runtime->stack_size exceeded")
    }

    runtime->stack[runtime->stack_size].lineno = lineno;
    runtime->stack[runtime->stack_size].timestamp = tl_time_epoch();

    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u16 i = 0;
        for ( ; i < 100 ; ++i) {
            if (function[i] == '\0') break;
            runtime->stack[runtime->stack_size].function[i] = function[i];
        }

        TLMEMSET(&runtime->stack[runtime->stack_size].function[i], 0, 100 - i);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
        TLMEMSET(runtime->stack[runtime->stack_size].arguments, 0, 1024);
    if (arguments != NULL){
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(runtime->stack[runtime->stack_size].arguments, 1024, arguments, arg_ptr);
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
        for ( ; i < 100 ; ++i) {
            if (filename[index + i] == '\0') break;
            runtime->stack[runtime->stack_size].filename[i] = filename[index + i];
        }

        TLMEMSET(&runtime->stack[runtime->stack_size].filename[i], 0, 100 - i);
    }

    runtime->stack_size++;
    if (runtime->stack_size > runtime->stack_maximum) {
        runtime->stack_maximum = runtime->stack_size;
    }
#endif
}

void tl_meta_frame_pop() {
#ifndef TELEIOS_BUILD_RELEASE
    if (runtime->stack_size == 0) TLWARN("runtime->stack_size is zero");
    runtime->stack_size--;
#endif
}


