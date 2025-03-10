#include "teleios/application.h"
#include "teleios/logger.h"
#include "teleios/container.h"
#include "teleios/platform.h"
#include "teleios/window.h"

TLWindow *window;

b8 tl_application_initialize(void) {
    TLTRACE("tl_application_initialize(void)")

    window = tl_window_create(1024, 768, "Teleios Application");
    if (window == NULL) {
        TLERROR("Failed to create window")
        return FALSE;
    }

    return TRUE;
}

b8 tl_application_run(void) {
    TLTRACE("tl_application_run(void)")
    TLList* list = tl_list_create();

    tl_list_destroy(list);
    list = NULL;

    return TRUE;
}

b8 tl_application_terminate(void) {
    tl_window_destroy(window);
    TLTRACE("tl_application_terminate(void)")
    return TRUE;
}