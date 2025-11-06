#include "teleios/teleios.h"
#include "string.h"

#if ! defined(TELEIOS_FRAME_MAXIMUM)
#   define TELEIOS_FRAME_MAXIMUM 10
#endif

#if ! defined(TL_PROFILER_FRAME_ARGUMENTS_SIZE)
#   define TL_PROFILER_FRAME_ARGUMENTS_SIZE 1024
#endif

typedef struct {
    const char* filename;
    const char* function;
    char arguments[TL_PROFILER_FRAME_ARGUMENTS_SIZE];
    u64 lineno;
} TLFrame;

static u8 frame_index = U8_MAX;
static TLFrame frame[TELEIOS_FRAME_MAXIMUM];

void tl_profiler_frame_push(const char* filename, const u64 lineno, const char* function, const char* arguments, ...) {
    frame_index++;

    // ✅ Branch hint: raramente estoura limite
    if (TL_UNLIKELY(frame_index >= TELEIOS_FRAME_MAXIMUM)) {
        TLFATAL("TELEIOS_FRAME_MAXIMUM of %d exceeded", TELEIOS_FRAME_MAXIMUM)
    }

    // ✅ Ponteiros diretos - ultra rápido
    frame[frame_index].filename = filename;
    frame[frame_index].function = function;
    frame[frame_index].lineno = lineno;

    // ✅ Branch hint: geralmente NULL (caso comum)
    if (TL_LIKELY(arguments == NULL)){ frame[frame_index].arguments[0] = '\0'; }
    else {
        va_list arg_ptr;
        va_start(arg_ptr, arguments);
        vsnprintf(frame[frame_index].arguments, TL_PROFILER_FRAME_ARGUMENTS_SIZE, arguments, arg_ptr);
        va_end(arg_ptr);
    }

    TLVERBOSE("[in ] %s(%s)", frame[frame_index].function, frame[frame_index].arguments)
}

void tl_profiler_frame_pop() {
    TLVERBOSE("[out] %s(%s)", frame[frame_index].function, frame[frame_index].arguments)
    memset(&frame[frame_index], 0 , sizeof(TLFrame));
    frame_index--;
}