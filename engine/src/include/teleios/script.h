#ifndef __TELEIOS_SCRIPT__
#define __TELEIOS_SCRIPT__

#include "teleios/defines.h"

b8 tl_script_initialize(void);
b8 tl_script_terminate(void);

b8 tl_script_execute(TLString* path);

#endif