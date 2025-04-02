#ifndef __TELEIOS_RUNTIME_SERIALIZER__
#define __TELEIOS_RUNTIME_SERIALIZER__

#include "teleios/defines.h"

/**
 * @brief Walk the global->yaml file invoking the processor with the configuration line in a java.properties style.
 * the processing function will be called with the prefix of the element, the element and it's value.
 *
 * @param processor The processing function
 */
void tl_serializer_walk(void (*processor)(const char *prefix, const char *element, const char *value));

#endif // __TELEIOS_RUNTIME_SERIALIZER__