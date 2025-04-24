#ifndef __TELEIOS_APPLICATION__
#define __TELEIOS_APPLICATION__

#include "teleios/defines.h"

typedef struct TLApplication TLApplication;

TLApplication* tl_application_get();

b8 tl_application_initialize(void);
b8 tl_application_run(void);
b8 tl_application_terminate(void);

#endif