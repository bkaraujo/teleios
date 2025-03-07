#include "teleios/platform.h"
#include "teleios/application.h"

int main (int argc, char *argv[]) {
    if (argc != 2) { TLFATAL("argc != 2"); }

    if (!tl_platform_initialize()) {
        TLERROR("Platform failed to intialize");
        if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
        exit(99);
    }

    if (!tl_application_initialize()) {
        TLERROR("Application failed to intialize");
        if (!tl_application_terminate()) { TLERROR("Application failed to terminate"); }
        if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
        exit(99);

    }
    
    if (!tl_application_run()) { TLERROR("Application failed to execute"); }
    if (!tl_application_terminate()) { TLERROR("Application failed to terminate"); }
    if (!tl_platform_terminate()) { TLFATAL("Platform failed to terminate"); }
}