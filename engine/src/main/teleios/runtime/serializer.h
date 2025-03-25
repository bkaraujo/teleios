#ifndef __TELEIOS_SERIALIZER__
#define __TELEIOS_SERIALIZER__

#include "teleios/runtime.h"

b8 tl_serializer_read_yaml(void);
b8 tl_serializer_read_scene(const char* name);

#endif // __TELEIOS_SERIALIZER__