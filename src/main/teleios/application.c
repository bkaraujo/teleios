#include "teleios/teleios.h"

struct TLApplication {
    KAllocator *allocator;
};

static TLApplication application = { 0 };

b8 tl_application_initialize(void) {
    K_FRAME_PUSH
    K_FRAME_POP_WITH(true)
}

TLApplication* tl_application_get() {
    K_FRAME_PUSH
    K_FRAME_POP_WITH(&application)
}

b8 tl_application_run(void) {
    K_FRAME_PUSH
    K_FRAME_POP_WITH(true)
}

b8 tl_application_terminate(void) {
    K_FRAME_PUSH
    K_FRAME_POP_WITH(true)
}