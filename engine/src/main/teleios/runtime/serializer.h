#ifndef __TELEIOS_SERIALIZER__
#define __TELEIOS_SERIALIZER__

#include "teleios/runtime.h"

void tl_serializer_read(void);
void tl_serializer_write(void);

TLScene* tl_serializer_scene(const char* name);

#endif // __TELEIOS_SERIALIZER__