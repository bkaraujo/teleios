#ifndef __TELEIOS_CONFIG__
#define __TELEIOS_CONFIG__

#include "teleios/defines.h"

b8 tl_config_initialize();
TLList* tl_config_list(TLString* property);
void* tl_config_get(const char* property);
TLLogLevel tl_config_get_log_level(const char* property);
TLDisplayResolution tl_config_get_display_resolution(const char* property);

b8 tl_config_terminate(void);

#endif