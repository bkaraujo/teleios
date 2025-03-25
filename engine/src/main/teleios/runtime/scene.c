#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

b8 tl_scene_load(const char* name) {
    TLSTACKPUSHA("%s", name)
    TLDEBUG("Loading scene [%s]", name);
    tl_serializer_scene(name);
    TLSTACKPOPV(TRUE)
}
