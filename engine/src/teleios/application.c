#include "teleios/application.h"
#include "teleios/logger.h"
#include "teleios/container.h"
#include "teleios/platform.h"
#include "GLFW/glfw3.h"

b8 tl_application_initialize(void) {
    TLTRACE(">> tl_application_initialize(void)")
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        return FALSE;
    }

    TLTRACE("<< tl_application_initialize(void)")
    return TRUE;
}

b8 tl_application_run(void) {
    TLTRACE(">> tl_application_run(void)")
    TLList* list = tl_list_create();

    tl_list_destroy(list);
    list = NULL;
    TLTRACE("<< tl_application_run(void)")
    return TRUE;
}

b8 tl_application_terminate(void) {
    TLTRACE(">> tl_application_terminate(void)")
    glfwTerminate();
    TLTRACE("<< tl_application_terminate(void)")
    return TRUE;
}