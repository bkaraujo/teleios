#include "teleios/core.h"

#if ! defined(TELEIOS_BUILD_RELEASE)
#include <stdio.h>
#include <stdarg.h>
#endif

void tl_meta_frame_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    if (core->stack_index >= sizeof(core->stack) / sizeof(TLStackFrame)) {
        TLFATAL("core->stack_index exceeded")
    }

    core->stack[core->stack_index].lineno = lineno;
    core->stack[core->stack_index].timestamp = tl_time_epoch();

    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    {
        u16 i = 0;
        for ( ; i < 100 ; ++i) {
            if (function[i] == '\0') break;
            core->stack[core->stack_index].function[i] = function[i];
        }

        TLMEMSET(&core->stack[core->stack_index].function[i], 0, 100 - i);
    }
    // ----------------------------------------------------------------
    // Copy the value and ensure the rest of the string is null
    // ----------------------------------------------------------------
    TLMEMSET(core->stack[core->stack_index].arguments, 0, 1024);
    if (arguments != NULL){
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(core->stack[core->stack_index].arguments, 1024, arguments, arg_ptr);
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
            core->stack[core->stack_index].filename[i] = filename[index + i];
        }

        TLMEMSET(&core->stack[core->stack_index].filename[i], 0, 100 - i);
    }

    core->stack_index++;
    if (core->stack_index > core->stack_maximum) {
        core->stack_maximum = core->stack_index;
    }

    // TLVERBOSE("STACK PUSH :: %s:%d %s(%s)",
    //     core->stack[core->stack_index].filename, core->stack[core->stack_index].lineno,
    //     core->stack[core->stack_index].function, core->stack[core->stack_index].arguments
    // )
#endif
}

void tl_meta_frame_pop() {
#if ! defined(TELEIOS_BUILD_RELEASE)
    if (core->stack_index == 0) TLWARN("core->stack_index is zero");

    // TLVERBOSE("STACK POP :: %s:%d %s(%s)",
    //     core->stack[core->stack_index].filename, core->stack[core->stack_index].lineno,
    //     core->stack[core->stack_index].function, core->stack[core->stack_index].arguments
    // )

    core->stack_index--;
#endif
}


