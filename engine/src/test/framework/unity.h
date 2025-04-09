#ifndef __TELEIOS_TEST_UNITY___
#define __TELEIOS_TEST_UNITY___

#include "teleios/core/logger.h"
#include <stdio.h>

#define TL_TEST_ASSERT(condition, message)                 \
    if (!(condition)) {                                    \
        KERROR("Teste falhou: %s na linha %d\n", message) \
        return 1; /* Indica falha no teste */              \
    }

#endif //__TELEIOS_TEST_UNITY___
