#ifndef __TELEIOS_CONFIG__
#define __TELEIOS_CONFIG__

#include "teleios/defines.h"

b8 tl_config_initialize();
void* tl_config_get(TLString* property);
TLList* tl_config_list(TLString* property);
b8 tl_config_terminate(void);

#endif