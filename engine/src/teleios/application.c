#include "teleios/application.h"
#include "teleios/logger.h"
#include "teleios/container.h"
#include "teleios/platform.h"

b8 tl_application_initialize(void) {
    TLTRACE("tl_application_initialize(void)")

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
    TLTRACE("tl_application_terminate(void)")
    return TRUE;
}