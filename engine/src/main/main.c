#include "teleios/teleios.h"
#include "teleios/application.h"

int main() {
    TLGlobal g = { 0 };
    global = &g;

    if (!tl_platform_initialize()) TLFATAL("Platform failed to initialize")
    if (!tl_application_initialize()) {
        if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")
        TLFATAL("Application failed to initialize")
    }

    if (!tl_application_run()) {
        if (!tl_application_terminate()) TLERROR("Platform failed to terminate")
        if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")
        TLFATAL("Application failed to run")
    }
    
    if (!tl_application_terminate()) TLERROR("Application failed to terminate")
    if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")

    return 0;
}