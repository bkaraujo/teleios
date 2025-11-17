#ifndef __TELEIOS_CONFIG__
#define __TELEIOS_CONFIG__

#include "teleios/defines.h"

b8 tl_config_initialize();
TLList* tl_config_list(TLString* property);

void* tl_config_get(const char* property);
b8 tl_config_get_b8(const char* property);
u8 tl_config_get_u8(const char* property);
u16 tl_config_get_u16(const char* property);
u32 tl_config_get_u32(const char* property);
u64 tl_config_get_u64(const char* property);
i8 tl_config_get_i8(const char* property);
i16 tl_config_get_i16(const char* property);
i32 tl_config_get_i32(const char* property);
i64 tl_config_get_i64(const char* property);
f32 tl_config_get_f32(const char* property);
f64 tl_config_get_f64(const char* property);
TLLogLevel tl_config_get_log_level(const char* property);
TLDisplayResolution tl_config_get_display_resolution(const char* property);

b8 tl_config_terminate(void);

#endif