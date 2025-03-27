#ifndef __TELEIOS_CORE_SERIALIZER__
#define __TELEIOS_CORE_SERIALIZER__

#include "teleios/defines.h"

void tl_serializer_walk(void (*processor)(const char *prefix, const char *element, const char *value, u64 length));

#endif // __TELEIOS_CORE_SERIALIZER__