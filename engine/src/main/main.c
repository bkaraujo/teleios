#include "teleios/teleios.h"
#include "teleios/application.h"

int main(const int argc, const char* argv[]) {
    (void)argc;  // Unused parameter
    (void)argv;  // Unused parameter
    tl_logger_set_level(TL_LOG_LEVEL_TRACE);
    // ####################################################################
    // 1 - Iniciar OS
    // ####################################################################
    if (!tl_platform_initialize()) TLFATAL("Platform failed to initialize")
    // ####################################################################
    // 2 - Carregar DLL Cliente
    // ####################################################################

    // ####################################################################
    // 3 - Iniciar Engine
    // ####################################################################
    if (!tl_application_initialize()) {
        if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")
        TLFATAL("Application failed to initialize")
    }
    // ####################################################################
    // 4 - Executar aplicação
    // ####################################################################
    if (!tl_application_run()) {
        if (!tl_application_terminate()) TLERROR("Platform failed to terminate")
        if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")
        TLFATAL("Application failed to run")
    }
    
    if (!tl_application_terminate()) TLERROR("Application failed to terminate")

    if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")

    return 0;
}