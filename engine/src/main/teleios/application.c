#include "teleios/teleios.h"
#include "teleios/application.h"

b8 tl_application_initialize(void) {
    TL_PROFILER_PUSH
    TLTRACE("tl_application_initialize()")
    TL_PROFILER_POP_WITH(true)
}

b8 tl_application_run(void) {
    TL_PROFILER_PUSH
    TLTRACE("tl_application_run()")
    TL_PROFILER_POP_WITH(true)
}

b8 tl_application_terminate(void) {
    TL_PROFILER_PUSH
    TLTRACE("tl_application_terminate()")
    TL_PROFILER_POP_WITH(true)
}