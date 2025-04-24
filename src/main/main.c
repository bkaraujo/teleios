#include "teleios/teleios.h"

int main (const int argc, const char *argv[]) {
    K_FRAME_PUSH_WITH("%i, 0%xp", argc, argv)

    if (argc != 2) { KERROR("argc != 2") K_FRAME_POP_WITH(99) }

    k_logger_loglevel(K_LOG_LEVEL_VERBOSE);

    KINFO("Initializing %s", argv[1]);

    if (!tl_runtime_initialize(argv[0])) { KERROR("Failed to initialize runtime"); return 99; }
    if (!tl_application_initialize()) {
        KERROR("Failed to initialize application");
        if (!tl_application_terminate()) KERROR("Failed to terminate application");
        if (!tl_runtime_terminate()) { KERROR("Failed to terminate runtime"); return 99; }
    }
    if (!tl_application_run()) KERROR("Failed to run application");
    if (!tl_application_terminate()) KERROR("Failed to terminate application");
    if (!tl_runtime_terminate()) { KERROR("Failed to terminate runtime"); return 99; }

    return 0;
}
