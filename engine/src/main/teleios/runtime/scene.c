#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

TLScene* tl_scene_create(const char* name) {
    TLSTACKPUSHA("%s", name)
    TLDEBUG("Loading scene %s", name);
    TLScene* scene = tl_serializer_scene(name);
    scene->arena = tl_memory_arena_create(TLMEBIBYTES(32));
    scene->name = tl_string_clone(scene->arena, name);
    TLSTACKPOPV(scene)
}
