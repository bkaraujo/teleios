#include <stdio.h>
#include "teleios/teleios.h"

int main(int argc, char* argv[]) {
    (void)argc;  // Unused parameter
    (void)argv;  // Unused parameter

    printf("Sandbox TELEIOS - Em Desenvolvimento\n");

    #ifdef TELEIOS_DEBUG
    printf("Modo DEBUG ativo\n");
    #endif

    return 0;
}
