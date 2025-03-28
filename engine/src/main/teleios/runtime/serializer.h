#ifndef __TELEIOS_RUNTIME_SERIALIZER__
#define __TELEIOS_RUNTIME_SERIALIZER__

#include "teleios/defines.h"

void tl_serializer_walk(void (*processor)(const char *prefix, const char *element, const char *value));

#endif // __TELEIOS_RUNTIME_SERIALIZER__